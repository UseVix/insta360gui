#ifndef INSTA360GUI__CAMERA_PERSPECTIVE_PANEL_HPP_
#define INSTA360GUI__CAMERA_PERSPECTIVE_PANEL_HPP_

#include <array>

#include <geometry_msgs/msg/quaternion.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rviz_common/panel.hpp>
#include <tf2/LinearMath/Quaternion.h>

class QLabel;
class QLineEdit;
class QSlider;

namespace insta360gui
{

/// RViz2 port of camera_perspective_slider.py.
///
/// Three sliders (roll / pitch / yaw, -180..180 deg). While a slider is dragged, the published
/// orientation is `committed * rotation(axis, slider_value)`. When the slider is released the
/// result becomes the new committed orientation and all sliders snap back to 0, so each drag is
/// an incremental rotation in the camera's local frame.
class CameraPerspectivePanel : public rviz_common::Panel
{
  Q_OBJECT

public:
  explicit CameraPerspectivePanel(QWidget * parent = nullptr);

  void onInitialize() override;
  void save(rviz_common::Config config) const override;
  void load(const rviz_common::Config & config) override;

private:
  void onSliderChanged(int axis, int degrees);
  void commit();
  void resetOrientation();
  void updateTopic();
  void publish(const tf2::Quaternion & q);
  void updateOrientationLabel();

  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<geometry_msgs::msg::Quaternion>::SharedPtr publisher_;

  QLineEdit * topic_edit_{nullptr};
  std::array<QSlider *, 3> sliders_{};
  std::array<QLabel *, 3> value_labels_{};
  QLabel * orientation_label_{nullptr};

  tf2::Quaternion committed_{0.0, 0.0, 0.0, 1.0};
  tf2::Quaternion current_{0.0, 0.0, 0.0, 1.0};
};

}  // namespace insta360gui

#endif  // INSTA360GUI__CAMERA_PERSPECTIVE_PANEL_HPP_
