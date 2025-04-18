import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Point
from std_msgs.msg import Bool
import math

class PositionPublisher(Node):
    def __init__(self):
        super().__init__('position_publisher')
        self.publisher_ = self.create_publisher(Point, 'target_position', 10)
        self.subscription = self.create_subscription(
                    Bool, 'trigger_position_publisher', self.trigger_callback, 10
                )

        # 動作パラメータ
        self.timer = None
        self.radius = 5.0  # 円の半径
        self.angular_speed = 2 * math.pi / 15.0  # 1周30秒で移動
        self.total_duration = 37.5  # 2.5周分の時間 (秒)
        self.linear_z_speed = -3.0 / self.total_duration  # z軸方向への移動速度 (2.5周で-3m)

        # 状態管理
        self.current_time = 0.0  # 経過時間
        self.position = Point()

        # 初期位置を設定 [0, 0, 5]
        self.position.x = 0.0  # 初期x座標
        self.position.y = 0.0  # 初期y座標
        self.position.z = 4.75  # 初期z座標

        # 現在の角度を初期位置に設定（開始時点で0度からスタート）
        self.angle = 0.0
        self.is_triggered = False 

    def trigger_callback(self, msg):
        if msg.data and not self.is_triggered:
            self.get_logger().info('Received trigger! Starting PositionPublisher.')
            self.is_triggered = True
            self.timer = self.create_timer(0.001, self.publish_position)

    def publish_position(self):
        # 現在の角度（時間から計算）
        self.angle = self.angular_speed * self.current_time

        # x-y平面の円軌道を計算 (開始点を[0, 0, 5]に設定)
        self.position.x = self.radius * math.cos(self.angle)-5.0
        self.position.y = self.radius * math.sin(self.angle)

        # z軸の線形移動を計算
        self.position.z += self.linear_z_speed * 0.001

        # 位置をパブリッシュ
        self.publisher_.publish(self.position)
        self.get_logger().info(f'Publishing: {self.position.x}, {self.position.y}, {self.position.z}')

        # 経過時間を更新
        self.current_time += 0.001

        # 2.5周分の時間を超えたら停止
        if self.current_time >= self.total_duration:
            self.get_logger().info('Completed 2.5 rounds. Stopping node.')
            rclpy.shutdown()

def main(args=None):
    rclpy.init(args=args)
    node = PositionPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
