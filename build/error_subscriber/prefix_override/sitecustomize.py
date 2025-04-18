import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/robotdeveloper02/IsaacSim-ros_workspaces-main/humble_ws/install/error_subscriber'
