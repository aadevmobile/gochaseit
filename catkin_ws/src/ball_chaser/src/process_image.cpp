#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <cmath>
//#include <cv_bridge/cv_bridge.h>


// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    
   ROS_INFO("Drive Robot Instructions: lin_x: %f, ang_z: %f", lin_x, ang_z);
    
   // Request a service and pass the velocities to it to drive the robot
   ball_chaser::DriveToTarget srv;
   srv.request.linear_x = lin_x;
   srv.request.angular_z = ang_z;
   // call 
   if (!client.call(srv))
      ROS_ERROR("Failed to call service command_robot");

}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

   int third = std::round(img.width/3);
   //ROS_INFO("Third of screen: [%d]",third);
   int pixel =0;
   int mod = 0;
   bool white_pixel = false;

   // Loop through each pixel in the image and check if there's a bright white one
   // Then, identify if this pixel falls in the left, mid, or right side of the image
   // Depending on the white ball position, call the drive_bot function and pass velocities to it
   // Request a stop when there's no white ball seen by the camera
   for (int i=0; i < img.height*img.step; i=i+3)
   {
      pixel++;
     // ROS_INFO("Pixel: [%i]", pixel);
     // ROS_INFO("i_value:%d", i);
     // ROS_INFO("data_value:%d", img.data[i]);

      if (img.data[i] == 255)
      {  
         ROS_INFO("data[%d]=255 -- checking other values",i);
	 if ( img.data[i+1] == 255 && img.data[i+2] == 255)
         {
            mod = pixel%img.width;
	    ROS_INFO_STREAM("White Pixel!");
            white_pixel = true;

            if (mod != 0 && mod < third+1){
               drive_robot(0.0, 0.2); //drive left
               ROS_INFO("Drive Left!");
            }
            if (mod !=0 && third < mod && third*2+1 > mod){
               drive_robot(0.2, 0.0); // drive straight
               ROS_INFO("Drive Straight!");
            }
            if (mod == 0 || mod > third*2){
	       drive_robot(0.0, -0.2); // drive right
               ROS_INFO("Drive Right!");
	    }

            break;
         }
      } 
   } if(!white_pixel)
	drive_robot(0.0,0.0); //halt

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
