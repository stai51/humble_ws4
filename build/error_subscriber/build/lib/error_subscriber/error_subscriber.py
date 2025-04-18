import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64

class ErrorSubscriber(Node):
    def __init__(self):
        super().__init__('error_subscriber')
        # "error_topic" というトピックをサブスクライブ
        self.subscription = self.create_subscription(
            Float64,
            'error_topic',
            self.error_callback,
            10
        )

    def error_callback(self, msg):
        # 受け取った誤差値を表示
        self.get_logger().info(f"Received error: {msg.data}")

def main(args=None):
    rclpy.init(args=args)
    node = ErrorSubscriber()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
