import rclpy
from geometry_msgs.msg import Twist
from omni.isaac.dynamic_control import _dynamic_control
import numpy as np
from rclpy.node import Node
import rclpy.subscription

class jetbotController(Node):
    def __init__(self):
        super().__init__("jetbot_controller")
        self.dc = _dynamic_control.acquire_dynamic_control_interface()
        self.joint_handle_left = self.dc.find_articulation_dof("jetbot_left_wheel_joint")
        self.joint_handle_right = self.dc.find_articulation_dof("jetbot_right_wheel_joint")
        self.subscription = self.create_subscription(
            Twist,
            "/jetbot/cmd_vel",
            self.cmd_vel_callback,
            10
        )
        self.subscription
        
    def cmd_vel_callback(self,msg):
        linear_x=msg.linear.x
        angular_z=msg.zngular.z

        wheel_speed_left=linear_x-angular_z
        wheel_speed_right=linear_x+angular_z

        self.dc.set_dof_velocity_target(self.joint_handle_left,wheel_speed_left)
        self.dc.set_dof_velocity_target(self.joint_handle_right,wheel_speed_right)

def main(args=None):
    rclpy.init(args=args)
    controller=jetbotController()
    rclpy.spin(controller)
    controller.destroy_node()
    rclpy.shutdown()

if __name__=='__main__':
    main()