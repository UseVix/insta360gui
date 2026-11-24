#include "insta360gui/camera_perspective_panel.hpp"

#include <cmath>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

#include <pluginlib/class_list_macros.hpp>
#include <rviz_common/display_context.hpp>
#include <tf2/LinearMath/Matrix3x3.h>

namespace insta360gui
{

namespace
{
constexpr char kDefaultTopic[] = "/camera_orientation/quaternion";
constexpr double kDegToRad = M_PI / 180.0;
constexpr double kRadToDeg = 180.0 / M_PI;
}  // namespace

CameraPerspectivePanel::CameraPerspectivePanel(QWidget * parent)
: rviz_common::Panel(parent)
{
  auto * layout = new QVBoxLayout(this);

  auto * topic_row = new QHBoxLayout();
  topic_row->addWidget(new QLabel("Topic:"));
  topic_edit_ = new QLineEdit(kDefaultTopic);
  topic_row->addWidget(topic_edit_);
  layout->addLayout(topic_row);

  auto * grid = new QGridLayout();
  const char * names[3] = {"Roll", "Pitch", "Yaw"};
  for (int axis = 0; axis < 3; ++axis) {
    auto * slider = new QSlider(Qt::Horizontal);
    slider->setRange(-180, 180);
    slider->setValue(0);
    slider->setPageStep(10);

    auto * value_label = new QLabel("0°");
    value_label->setMinimumWidth(40);
    value_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    grid->addWidget(new QLabel(names[axis]), axis, 0);
    grid->addWidget(slider, axis, 1);
    grid->addWidget(value_label, axis, 2);

    sliders_[axis] = slider;
    value_labels_[axis] = value_label;

    connect(slider, &QSlider::valueChanged, this, [this, axis](int v) {onSliderChanged(axis, v);});
    connect(slider, &QSlider::sliderReleased, this, [this]() {commit();});
  }
  layout->addLayout(grid);

  orientation_label_ = new QLabel();
  orientation_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  layout->addWidget(orientation_label_);

  auto * reset_button = new QPushButton("Reset orientation");
  layout->addWidget(reset_button);
  layout->addStretch();

  connect(reset_button, &QPushButton::clicked, this, [this]() {resetOrientation();});
  connect(topic_edit_, &QLineEdit::editingFinished, this, [this]() {updateTopic();});

  updateOrientationLabel();
}

void CameraPerspectivePanel::onInitialize()
{
  node_ = getDisplayContext()->getRosNodeAbstraction().lock()->get_raw_node();
  updateTopic();
}

void CameraPerspectivePanel::onSliderChanged(int axis, int degrees)
{
  value_labels_[axis]->setText(QString("%1°").arg(degrees));

  tf2::Vector3 rotation_axis(axis == 0, axis == 1, axis == 2);
  tf2::Quaternion delta(rotation_axis, degrees * kDegToRad);
  current_ = (committed_ * delta).normalized();
  publish(current_);
  updateOrientationLabel();

  // Changes not made by dragging the handle (mouse wheel, arrow keys, clicking the groove) have no
  // "release" event, so apply them immediately as a single increment.
  if (!sliders_[axis]->isSliderDown()) {
    commit();
  }
}

void CameraPerspectivePanel::commit()
{
  committed_ = current_;
  for (int axis = 0; axis < 3; ++axis) {
    const QSignalBlocker blocker(sliders_[axis]);
    sliders_[axis]->setValue(0);
    value_labels_[axis]->setText("0°");
  }
}

void CameraPerspectivePanel::resetOrientation()
{
  committed_ = tf2::Quaternion(0.0, 0.0, 0.0, 1.0);
  current_ = committed_;
  commit();
  publish(current_);
  updateOrientationLabel();
}

void CameraPerspectivePanel::updateTopic()
{
  if (!node_) {
    return;  // Called from load() before onInitialize(); onInitialize() will create it.
  }
  const std::string topic = topic_edit_->text().trimmed().toStdString();
  if (topic.empty()) {
    publisher_.reset();
    return;
  }
  if (publisher_ && publisher_->get_topic_name() == node_->get_node_topics_interface()
    ->resolve_topic_name(topic))
  {
    return;
  }
  publisher_ = node_->create_publisher<geometry_msgs::msg::Quaternion>(topic, 10);
  Q_EMIT configChanged();
}

void CameraPerspectivePanel::publish(const tf2::Quaternion & q)
{
  if (!publisher_) {
    return;
  }
  geometry_msgs::msg::Quaternion msg;
  msg.x = q.x();
  msg.y = q.y();
  msg.z = q.z();
  msg.w = q.w();
  publisher_->publish(msg);
}

void CameraPerspectivePanel::updateOrientationLabel()
{
  double roll, pitch, yaw;
  tf2::Matrix3x3(current_).getRPY(roll, pitch, yaw);
  orientation_label_->setText(
    QString("q = [%1, %2, %3, %4]\nRPY = [%5°, %6°, %7°]")
    .arg(current_.x(), 0, 'f', 3).arg(current_.y(), 0, 'f', 3)
    .arg(current_.z(), 0, 'f', 3).arg(current_.w(), 0, 'f', 3)
    .arg(roll * kRadToDeg, 0, 'f', 1).arg(pitch * kRadToDeg, 0, 'f', 1)
    .arg(yaw * kRadToDeg, 0, 'f', 1));
}

void CameraPerspectivePanel::save(rviz_common::Config config) const
{
  rviz_common::Panel::save(config);
  config.mapSetValue("Topic", topic_edit_->text());
}

void CameraPerspectivePanel::load(const rviz_common::Config & config)
{
  rviz_common::Panel::load(config);
  QString topic;
  if (config.mapGetString("Topic", &topic)) {
    topic_edit_->setText(topic);
    updateTopic();
  }
}

}  // namespace insta360gui

PLUGINLIB_EXPORT_CLASS(insta360gui::CameraPerspectivePanel, rviz_common::Panel)
