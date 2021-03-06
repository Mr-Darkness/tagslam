/* -*-c++-*--------------------------------------------------------------------
 * 2018 Bernd Pfrommer bernd.pfrommer@gmail.com
 */

#include "tagslam/yaml_utils.h"
#include <XmlRpcException.h>

namespace tagslam {
  static double read_field(XmlRpc::XmlRpcValue v) {
    double x = 0;
    try {
      x = static_cast<double>(v);
    } catch (const XmlRpc::XmlRpcException &e) {
      x = (double)static_cast<int>(v);
    }
    return (x);
  }
  namespace yaml_utils {
    Eigen::Vector3d get_vec(const std::string &name,
                            XmlRpc::XmlRpcValue v) {
      try {
        double x(0), y(0), z(0);
        for (XmlRpc::XmlRpcValue::iterator it = v.begin();
             it != v.end(); ++it) {
          std::string field = it->first;
          if (field == "x") {        x = read_field(it->second);
          } else if (field == "y") { y = read_field(it->second);
          } else if (field == "z") { z = read_field(it->second);
          }
        }
        return (Eigen::Vector3d(x, y, z));
      } catch (const XmlRpc::XmlRpcException &e) {
        throw std::runtime_error("error parsing vector: " + name);
      }
    }

    bool get_pose_and_noise(XmlRpc::XmlRpcValue pose_and_noise,
                            gtsam::Pose3 *pose, PoseNoise *noise,
                            double defPosNoise, double defRotNoise) {
      Eigen::Vector3d anglevec, center, rotnoise, posnoise;
      int nfound(0);
      bool foundRotNoise(false), foundPosNoise(false);
      for (XmlRpc::XmlRpcValue::iterator it = pose_and_noise.begin();
           it != pose_and_noise.end(); ++it) {
        if (it->first == "rotvec") {
          anglevec = get_vec("rotvec", it->second);
          nfound++;
        } else if (it->first == "center") {
          center   = get_vec("center", it->second);
          nfound++;
        } else if (it->first == "rotation_noise") {
          rotnoise = get_vec("rotation_noise", it->second);
          foundRotNoise = true;
        } else if (it->first == "position_noise") {
          posnoise = get_vec("position_noise", it->second);
          foundPosNoise = true;
        }
      }
      gtsam::Rot3   R(utils::rotmat(anglevec));
      gtsam::Point3 T(center);
      *pose = gtsam::Pose3(R, T);
      if (!foundRotNoise) {
        rotnoise = gtsam::Vector(3);
        rotnoise << defRotNoise, defRotNoise, defRotNoise;
      }
      if (!foundPosNoise) {
        posnoise = gtsam::Vector(3);
        posnoise << defPosNoise, defPosNoise, defPosNoise;
      }
      *noise = makePoseNoise(rotnoise, posnoise);
      return (nfound == 2);
    }

  }
}  // namespace
