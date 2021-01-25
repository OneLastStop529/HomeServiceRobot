#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include "nav_msgs/Odometry.h"
#include <cmath>

double pickupPos[2] = {5.9, 3.5};
double dropoffPos[2] = {-1.3, -5.0};

double pose[2] = {0,0};

void get_current_pose(const nav_msgs::Odometry::ConstPtr& msg) {
  pose[0] = msg->pose.pose.position.x;
  pose[1] = msg->pose.pose.position.y;
}



double distToPosition(double goal[2]) {
  double dx = goal[0] - pose[0];
  double dy = goal[1] - pose[1];
  return sqrt(dx*dx + dy*dy);
}

bool reach_pickup() {
  return distToPosition(pickupPos) < 0.5;
}

bool reach_dropoff() {
  return distToPosition(dropoffPos) < 0.5;
}



int main( int argc, char** argv )
{
  ros::init(argc, argv, "add_markers");
  ros::NodeHandle n;
  ros::Rate r(1);
  ros::Publisher marker_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 1);
  ros::Subscriber pose_sub = n.subscribe("odom", 10, get_current_pose);
  // Set our initial shape type to be a cube
  uint32_t shape = visualization_msgs::Marker::CUBE;
  
  enum State {
    PICKUP,
    REACHED,
    DROPOFF,
  } state = PICKUP;

  while (ros::ok())
  {
    visualization_msgs::Marker marker;
    // Set the frame ID and timestamp.  See the TF tutorials for information on these.
    marker.header.frame_id = "/map";
    marker.header.stamp = ros::Time::now();

    // Set the namespace and id for this marker.  This serves to create a unique ID
    // Any marker sent with the same namespace and id will overwrite the old one
    marker.ns = "basic_shapes";
    marker.id = 0;

    // Set the marker type.  Initially this is CUBE, and cycles between that and SPHERE, ARROW, and CYLINDER
    marker.type = shape;

    // Set the marker action.  Options are ADD, DELETE, and new in ROS Indigo: 3 (DELETEALL)
    marker.action = visualization_msgs::Marker::ADD;

    // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
    marker.pose.position.x = 0;
    marker.pose.position.y = 0;
    marker.pose.position.z = 0;
    marker.pose.orientation.x = 0.0;
    marker.pose.orientation.y = 0.0;
    marker.pose.orientation.z = 0.0;
    marker.pose.orientation.w = 1.0;

    // Set the scale of the marker -- 1x1x1 here means 1m on a side
    marker.scale.x = 1.0;
    marker.scale.y = 1.0;
    marker.scale.z = 1.0;

    // Set the color -- be sure to set alpha to something non-zero!
    marker.color.r = 0.0f;
    marker.color.g = 1.0f;
    marker.color.b = 0.0f;
    marker.color.a = 1.0;

    marker.lifetime = ros::Duration();

    // Publish the marker
    // while (marker_pub.getNumSubscribers() < 1)
    // {
    //   if (!ros::ok())
    //   {
    //     return 0;
    //   }
    //   ROS_WARN_ONCE("Please create a subscriber to the marker");
    //   sleep(1);
    // }
    ros::spinOnce();
    
    if (state == PICKUP) {
      marker.action = visualization_msgs::Marker::ADD;
      marker.pose.position.x = pickupPos[0];
      marker.pose.position.y = pickupPos[1];
      marker_pub.publish(marker);
      if (reach_pickup()) {
        sleep(5);
        ROS_INFO("Picked up Object, now heading to dropoff zone...");
        state = REACHED;
      }
    } else if (state == REACHED) {
      marker.action = visualization_msgs::Marker::DELETE;
      marker.pose.position.x = dropoffPos[0];
      marker.pose.position.y = dropoffPos[1];
      marker_pub.publish(marker);
      if (reach_pickup()) {
        sleep(5);
        ROS_INFO("Reached Dropoff zone");
        state = DROPOFF;
      }
    } else {
      marker.action = visualization_msgs::Marker::ADD;
      marker.pose.position.x = dropoffPos[0];
      marker.pose.position.y = dropoffPos[1];
      marker_pub.publish(marker);
    }
    

    
    r.sleep();
  }
}