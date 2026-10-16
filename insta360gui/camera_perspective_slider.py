import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Quaternion
import tkinter as tk
from tf_transformations import quaternion_multiply
import numpy as np
class CameraOrientationPublisher(Node):
    def __init__(self):
        super().__init__('camera_orientation_publisher')
        
        # Create the publisher
        self.publisher_ = self.create_publisher(Quaternion, '/camera_orientation/quaternion', 10)

        # Initialize the Tkinter GUI
        self.root = tk.Tk()
        self.root.title("Camera Orientation Sliders")
        self.root.geometry("300x200")

        # Create 3 sliding bars (trackbars)
        # The 'command' argument triggers the publish_callback whenever a slider is moved
        self.slider1 = tk.Scale(
            self.root, from_=-180, to=180, orient=tk.HORIZONTAL, label="Slider 1 (Roll)", 
            command=lambda val: self.publish_callback(val, slider_name=0)
        )
        self.slider1.pack(fill=tk.X, padx=10, pady=5)

        self.slider2 = tk.Scale(
            self.root, from_=-180, to=180, orient=tk.HORIZONTAL, label="Slider 2 (Pitch)", 
            command=lambda val: self.publish_callback(val, slider_name=1)
        )
        self.slider2.pack(fill=tk.X, padx=10, pady=5)

        self.slider3 = tk.Scale(
            self.root, from_=-180, to=180, orient=tk.HORIZONTAL, label="Slider 3 (Yaw)", 
            command=lambda val: self.publish_callback(val, slider_name=2)
        )
        self.slider3.pack(fill=tk.X, padx=10, pady=5)
        # Start the loop that keeps ROS spinning while the Tkinter GUI is active
        self.root.after(50, self.spin_ros)
        self.Camera_Orientatation_Quaternion = Quaternion(x=0.0,y=0.0,z=0.0,w=1.0)

        # Track left mouse button state
        self.root.bind_all('<ButtonRelease-1>', self.on_button_release)
    def publish_callback(self, val,slider_name):
        """Called automatically every time a slider is moved."""
        
        # 1. Variables with the value of the trackbars
        val1 = float(self.slider1.get())
        val2 = float(self.slider2.get())
        val3 = float(self.slider3.get())

        q_list1 = [self.Camera_Orientatation_Quaternion.x, self.Camera_Orientatation_Quaternion.y, self.Camera_Orientatation_Quaternion.z, self.Camera_Orientatation_Quaternion.w]
        q_list2 = [0.0]*4
        q_list2[slider_name] = np.sin(np.deg2rad(float(val))/2) 
        q_list2[3] = np.cos(np.deg2rad(float(val))/2)

        # 3. Multiply them
        self.result_list = quaternion_multiply(q_list1, q_list2)
        msg = Quaternion(
                x=float(self.result_list[0]),
                y=float(self.result_list[1]),
                z=float(self.result_list[2]),
                w=float(self.result_list[3])
            )
        self.publisher_.publish(msg)



    def on_button_release(self, event):
        """Track when mouse button 1 is released and snap sliders back to 0."""
        self.slider1.set(0)
        self.slider2.set(0)
        self.slider3.set(0)
        self.Camera_Orientatation_Quaternion.x = self.result_list[0]
        self.Camera_Orientatation_Quaternion.y = self.result_list[1]
        self.Camera_Orientatation_Quaternion.z = self.result_list[2]
        self.Camera_Orientatation_Quaternion.w = self.result_list[3]

    def spin_ros(self):
        """Non-blocking ROS spin to keep the node alive alongside Tkinter."""
        rclpy.spin_once(self, timeout_sec=0.01)
        self.root.after(50, self.spin_ros)

def main(args=None):
    rclpy.init(args=args)
    node = CameraOrientationPublisher()
    
    try:
        # Start the Tkinter main loop
        node.root.mainloop()
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
