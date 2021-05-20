#include <object_global_localizator.h>

sensor_msgs::NavSatFix global_position;
nav_msgs::Odometry local_position;
//std_msgs::Int8 objects_in_camera_view;
ros::Publisher object_global_position_pub;
darknet_ros_msgs::BoundingBoxes boundingBoxes;

bool globalPosGlobalFlag;
bool globalPosLocalFlag;
bool boundingBoxesFlag;

static Eigen::Matrix<double, 3, 3> cameraRotation;
static Eigen::Matrix<double, 3, 3> droneRotation;

void global_pos_cb(const sensor_msgs::NavSatFix::ConstPtr& msg){
    global_position = *msg;
    globalPosGlobalFlag = true;
    //ROS_INFO("global pos read");
}

void local_pos_cb(const nav_msgs::Odometry::ConstPtr& msg){
    local_position = *msg;
    globalPosLocalFlag = true;
    //ROS_INFO("local pos read");
}

/*void object_detector_cb(const std_msgs::Int8::ConstPtr& msg){
    objects_in_camera_view = *msg;
}*/

void bounding_boxes_cb(const darknet_ros_msgs::BoundingBoxes::ConstPtr& msg){
    boundingBoxes = *msg;
    boundingBoxesFlag = true;
    //ROS_INFO("boundingBoxes read");
}

void setup_camera_rotation(double pitch){
    Eigen::Matrix<double, 3, 3> rotation_Z;
    Eigen::Matrix<double, 3 ,3> rotation_Y;
    rotation_Y  <<      cos(pitch),     0,  sin(pitch),
                        0,              1,  0,
                        -sin(pitch),    0,  cos(pitch);
    double zRotation = M_PI/2;
    rotation_Z <<   cos(zRotation),     -sin(zRotation),    0,
                    sin(zRotation),     cos(zRotation),     0,
                    0,                  0,                  1;

    cameraRotation = rotation_Y * rotation_Z;
    //cameraRotation = cameraRotation.transpose();
}

void init_publisher(ros::NodeHandle controlNode){
    object_global_position_pub = controlNode.advertise<object_global_localizator_msgs::ObjectsGlobalPositions>("/objec_global_localizator", 1);
}

void localizeObjects(){
    object_global_localizator_msgs::ObjectsGlobalPositions outMessage;
    for (const auto & bounding_boxe : boundingBoxes.bounding_boxes) {
        object_global_localizator_msgs::ObjectGlobalPosition objectGlobalPosition;
        objectGlobalPosition.classObject = bounding_boxe.Class;
        objectGlobalPosition.idClassObject = bounding_boxe.id;
        objectGlobalPosition.probabilityObject = bounding_boxe.probability;
        double xObjCamera = -double(double(bounding_boxe.xmin) + (double(bounding_boxe.xmax - bounding_boxe.xmin) / 2) - CAMERA_X_CENTER);
        double yObjCamera = -(double(bounding_boxe.ymin) + (double(bounding_boxe.ymax - bounding_boxe.ymin) / 2) - CAMERA_Y_CENTER);
        double gamma = (xObjCamera / CAMERA_X_MAX) * CAMERA_X_ANGLE;
        double beta = (yObjCamera / CAMERA_Y_MAX) * CAMERA_Y_ANGLE;
        Eigen::Matrix<double, 3, 1> scalarToObjCenter_Camera;
        double zScalarObj = sqrt(1 / (1 + pow(tan(gamma),2) * pow(cos(beta),2) + pow(tan(beta),2)));
        double yScalarObj = tan(beta) * zScalarObj;
        double xScalarObj = tan(gamma) * cos(beta) * zScalarObj;
        scalarToObjCenter_Camera << xScalarObj, yScalarObj, zScalarObj;
        Eigen::Matrix<double, 3, 1> scalarToObjCenter_Global;
        scalarToObjCenter_Global = droneRotation * cameraRotation * scalarToObjCenter_Camera;
        double scale = abs(local_position.pose.pose.position.z / scalarToObjCenter_Global[2]);
        Eigen::Matrix<double, 3, 1> objectLocalPositionVector;
        objectLocalPositionVector = scalarToObjCenter_Global * scale;
        objectGlobalPosition.globalPositionLocal.x = objectLocalPositionVector[0] + local_position.pose.pose.position.x;
        objectGlobalPosition.globalPositionLocal.y = objectLocalPositionVector[1] + local_position.pose.pose.position.y;
        objectGlobalPosition.globalPositionLocal.z = 0;
        objectGlobalPosition.distanceDroneToObject = sqrt(pow(objectLocalPositionVector[0], 2)
                                                          + pow(objectLocalPositionVector[1], 2)
                                                          + pow(objectLocalPositionVector[2], 2));
        objectGlobalPosition.altitude = global_position.altitude - local_position.pose.pose.position.z;
        objectGlobalPosition.longitude = global_position.longitude + (360.0 / 40075000 / cos(global_position.latitude * M_PI / 360) * objectLocalPositionVector[0]);
        objectGlobalPosition.latitude = global_position.latitude + objectLocalPositionVector[1] * MERES_TO_LATITUDE;

        outMessage.ObjectsGlobalPositions.push_back(objectGlobalPosition);
    }
    object_global_position_pub.publish(outMessage);

}

void resetFlags()
{
    globalPosGlobalFlag = false;
    globalPosLocalFlag = false;
    boundingBoxesFlag = false;
}

bool checkFlags()
{
    return globalPosGlobalFlag && globalPosLocalFlag && boundingBoxesFlag;
}

void setDroneRotationMatrix()
{
    double x = local_position.pose.pose.orientation.x;
    double y = local_position.pose.pose.orientation.y;
    double z = local_position.pose.pose.orientation.z;
    double w = local_position.pose.pose.orientation.w;
    droneRotation <<    (1 - 2*pow(y,2) - 2*pow(z,2)),  (2*x*y - 2*z*w),    (2*x*z + 2*y*w),
                        (2*x*y + 2*z*w),    (1 - 2*pow(x,2) - 2*pow(z,2)),  (2*y*z - 2*x*w),
                        (2*x*z - 2*y*w),    (2*y*z + 2*x*w),    (1 - 2*pow(x,2) - 2*pow(y,2));
}