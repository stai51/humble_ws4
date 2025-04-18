import rclpy
from rclpy.node import Node
from geometry_msgs.msg import PoseStamped
import numpy as np

class PoseSubscriber(Node):
    def __init__(self):
        super().__init__('PoseStamped_subscriber')
        self.subscription1=self.create_subscription(
            PoseStamped,
            'Falling_Cube',
            self.listener_callback_1,
            10)
        self.subscription2=self.create_subscription(
            PoseStamped,
            'Fixed_Cube',
            self.listener_callback_2,
            10)

    def listener_callback_1(self,msg):
        position=msg.pose.position
        orientation=msg.pose.orientation
        self.get_logger().info(f'Received Falling_Cube_Pose:Position({position.x},{position.y},{position.z}),'
#                               f'orientation({orientation.x},{orientation.y},{orientation.z},{orientation.w})'
                                )
    def listener_callback_2(self,msg):
        position=msg.pose.position
        orientation=msg.pose.orientation
        self.get_logger().info(f'Received Fixed_Cube_Pose:Position({position.x},{position.y},{position.z}),'
#                               f'orientation({orientation.x},{orientation.y},{orientation.z},{orientation.w})'
                                )


def main(args=None):
    rclpy.init(args=args)
    pose_subscriber=PoseSubscriber()
    rclpy.spin(pose_subscriber)
    pose_subscriber.destroy_node()
    rclpy.shutdown()

if __name__=='__main__':
    main()