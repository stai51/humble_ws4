import rclpy
from rclpy.node import Node
from std_msgs.msg import String
from std_msgs.msg import String, Float64, Bool  # Bool メッセージを追加

class JettbotPosition(Node):
    def __init__(self):
        super().__init__('jettbot_position')

        # サブスクライバー（Jettbot位置情報を取得）
        self.subscription = self.create_subscription(
            String,  # トピックの型
            'End_Effector',  # トピック名
            self.listener_callback,  # コールバック関数
            10  # キューサイズ
        )

        # パブリッシャー（位置情報を出力）
        self.publisher_ = self.create_publisher(Float64, 'stop_jettbot_position', 10)
        # トリガー用
        self.trigger_publisher = self.create_publisher(Bool, 'trigger_position_publisher', 10)  

        # 状態変数
        self.is_fixed = False  # 初期状態では固定していない
        self.fixed_output = 0.0  # 固定出力
        self.count = 0  # メッセージ受信回数
        self.ignore_threshold = 1000  # 無視する受信回数
        self.trigger_threshold = 6300


    def listener_callback(self, msg):
        # メッセージの内容をパース
        data = msg.data.split(',')
        num=0#円なら0直線なら1
        if len(data) != 16:
            self.get_logger().error('Invalid data format received. Expected 16 elements.')
            return

        try:
            # x, y, z 座標を抽出（M, N, Oに対応するデータ）
            x = float(data[12])  # M
            y = float(data[13])  # N
            z = float(data[14])  # O

            # ログに表示
            self.get_logger().info(f'Received position: x={x}, y={y}, z={z}, count={self.count}')
            self.count += 1  # 受信回数を増加

            if num==0:
                if not self.is_fixed:#円用
                    # 最初の1000回は固定しない
                    if self.count > self.ignore_threshold:
                        # Jettbotが固定位置に近いか判定
                        error = abs(x - 0.0) + abs(y - -10.0) + abs(z - 0.99)  # 位置誤差の計算
                        if error < 0.05:  # 誤差がしきい値以下なら
                            self.get_logger().info('Jettbot reached the target area. Outputting fixed value.')
                            self.is_fixed = True  # 固定状態に変更

            if num==1:
                if not self.is_fixed:#直線用
                    # 最初の1000回は固定しない
                    if self.count > self.ignore_threshold:
                        # Jettbotが固定位置に近いか判定
                        error = abs(y - 10.0)
                        if error < 0.05:  # 誤差がしきい値以下なら
                            self.get_logger().info('Jettbot reached the target area. Outputting fixed value.')
                            self.is_fixed = True  # 固定状態に変更


            # 現在の状態に応じて出力
            if self.is_fixed :
                output_value = 0.0 
            else:
                if num==0: 
                    output_value = 6.283  # 位置が固定されたら0.0、そうでなければ6.283を出力
                if num==1:
                    output_value = 3.0  # 位置が固定されたら0.0、そうでなければ6.283を出力(直線)
            self.publisher_.publish(Float64(data=output_value))
            # 一定回数を超えたらトリガーをパブリッシュ**
            if self.count >= self.trigger_threshold:
                self.get_logger().info('Triggering PositionPublisher!')
                self.trigger_publisher.publish(Bool(data=True))

        except ValueError as e:
            self.get_logger().error(f'Failed to parse position: {e}')

def main(args=None):
    rclpy.init(args=args)
    node = JettbotPosition()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
