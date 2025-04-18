#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/int32.hpp"
#include "custom_message/msg/imu_msg.hpp"
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include "visualization_msgs/msg/marker.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include <vector>
#include <random>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <queue>
#include <regex>
#include <sstream>
#include <deque>
#include <limits>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <iostream>

using Eigen::Vector3d;
using Eigen::Quaterniond;

class calc:public rclcpp::Node
{
    public:

    calc()
    :Node("Multi_pose_subscriber"), particle_count_(125), received_all_(false) 
    {
        subscription1_=this->create_subscription<std_msgs::msg::String>(
        "End_Effector",10,std::bind(&calc::topic1_callback,this,std::placeholders::_1)); 

        subscription2_=this->create_subscription<geometry_msgs::msg::PoseStamped>(
        "Anchor_1_position",10,std::bind(&calc::topic2_callback,this,std::placeholders::_1));

        subscription3_=this->create_subscription<geometry_msgs::msg::PoseStamped>(
        "Anchor_2_position",10,std::bind(&calc::topic3_callback,this,std::placeholders::_1));

        subscription4_=this->create_subscription<geometry_msgs::msg::PoseStamped>(
        "Anchor_3_position",10,std::bind(&calc::topic4_callback,this,std::placeholders::_1));

        subscription5_=this->create_subscription<geometry_msgs::msg::PoseStamped>(
        "Anchor_4_position",10,std::bind(&calc::topic5_callback,this,std::placeholders::_1));

        subscription6_=this->create_subscription<geometry_msgs::msg::PoseStamped>(
        "Anchor_5_position",10,std::bind(&calc::topic6_callback,this,std::placeholders::_1));

        imu_subscriber5_ = this->create_subscription<custom_message::msg::ImuMsg>(
        "IMU_data5", 10, std::bind(&calc::imu_callback5, this, std::placeholders::_1));

        init_particles(5);

        last_time = rclcpp::Time(0);  // 初期値として0を設定
        
        estimated_positions_history.resize(5);  // NUM_ANCHORS: アンカーの総数    

        previousEllipses_.resize(5);  // NUM_ANCHORS: アンカーの総数

        previous_errors_.resize(5);

        x = Eigen::VectorXd::Zero(10);

        estimated_pose_publisher_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("estimated_pose", 10);

        for (size_t i = 0; i < 5; ++i) {
            marker_publishers_[i] = this->create_publisher<visualization_msgs::msg::Marker>(
                "estimated_position_marker_" + std::to_string(i), 10);
        }

        robot_marker_publisher_ = this->create_publisher<visualization_msgs::msg::Marker>("robot_marker", 10);
        anchor4_marker_publisher_ = this->create_publisher<visualization_msgs::msg::Marker>("anchor4_marker", 10);

        for (size_t i = 0; i < 5; ++i) {
            particle_marker_publishers_[i] = this->create_publisher<visualization_msgs::msg::MarkerArray>(
                "particle_markers_" + std::to_string(i), 10);
        }
                // パブリッシャーの作成
        time_publisher_ = this->create_publisher<std_msgs::msg::Int32>("time_axis", 10);
        anchor_error_publishers_[0] = this->create_publisher<std_msgs::msg::Float64>("/error/anchor_0", 10);
        anchor_error_publishers_[1] = this->create_publisher<std_msgs::msg::Float64>("/error/anchor_1", 10);
        anchor_error_publishers_[2] = this->create_publisher<std_msgs::msg::Float64>("/error/anchor_2", 10);
        anchor_error_publishers_[3] = this->create_publisher<std_msgs::msg::Float64>("/error/anchor_3", 10);
        anchor_error_publishers_[4] = this->create_publisher<std_msgs::msg::Float64>("/error/anchor_4", 10);
    }
    private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription1_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription2_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription3_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription4_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription5_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr subscription6_;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr estimated_pose_publisher_;
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr marker_publishers_[5];  // 5つのアンカー用パブリッシャー
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr robot_marker_publisher_;
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr anchor4_marker_publisher_;
    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr particle_marker_publishers_[5];
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr time_publisher_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr anchor_error_publishers_[5];  // アンカーごとの誤差をパブリッシュ
    rclcpp::Subscription<custom_message::msg::ImuMsg>::SharedPtr imu_subscriber1_;
    rclcpp::Subscription<custom_message::msg::ImuMsg>::SharedPtr imu_subscriber2_;
    rclcpp::Subscription<custom_message::msg::ImuMsg>::SharedPtr imu_subscriber3_;
    rclcpp::Subscription<custom_message::msg::ImuMsg>::SharedPtr imu_subscriber4_;
    rclcpp::Subscription<custom_message::msg::ImuMsg>::SharedPtr imu_subscriber5_;
    int gizi=0;//疑似アンカーありなら1なしなら0
    Eigen::VectorXd x;  // 状態ベクトル
    size_t anchor_idx = 4;  // アンカーインデックス
    geometry_msgs::msg::Point p;  // 現在の位置
    int timer=0;
    rclcpp::Time last_time;
    double imu_dt; 
    tf2::Vector3 Anchor_positions[5];  // 5つのアンカーの位置 (事前に設定された位置)
    std::vector<tf2::Vector3> robot_positions_;
    tf2::Vector3 Robot_anchor_positions[5];  // 5つのロボットアンカー位置を格納する配列
    tf2::Vector3 Robot_position;
    int marker_id_counter_ = 0;  // マーカーIDのカウンター
    std::queue<int> marker_history_;  // 表示中のマーカーIDの履歴
    const int max_markers_ = 100;      // 同時に表示する最大マーカー数
    int robot_marker_id_counter_ = 0;  // マーカーIDのカウンター
    std::queue<int> robot_marker_history_;  // 表示中のマーカーIDの履歴
    std::queue<int> anchor4_marker_history_;  // 表示中のマーカーIDの履歴
    const int robot_max_markers_ = 100;      // 同時に表示する最大マーカー数
    const int anchor4_max_markers_ = 100;      // 同時に表示する最大マーカー数
    double Robot[3];
    int anchor_move[5];
    int anchor_estimation_complete[5];
    int available_anchor[5];
    int is_resampling_started[5]={0,0,0,0,0};
    int time[5]={0,0,0,0,0};
    double max_weight[5];
    bool received[7] = { false, false, false, false, false,false,false};
    double Error;
    std::vector<double> previous_errors_;
    std::string position;
    size_t particle_count_;
    bool received_all_;
    struct Particle {
        tf2::Vector3 position;
        double weight;
    };
    std::vector<std::deque<tf2::Vector3>> estimated_positions_history;// アンカーごとの推定履歴
    std::vector<tf2::Vector3> anchor_positions_at_6300;
    const size_t max_history_size = 50;  // 直近50回分を保持
    std::vector<Particle> particles_;
    std::vector<std::vector<Particle>> particle_groups_; 
    double dt;
    Vector3d imu_acc;  // IMUからの加速度
    Vector3d imu_gyro; // IMUからの角速度
    // 楕円データを保持する構造体
    struct Ellipse {
        Eigen::Vector3d center;             // 楕円の中心
        Eigen::Vector3d side;               // 楕円の各軸長 (長辺 a, 中辺 b, 短辺 c)
        Eigen::Matrix3d eigVec;            // 固有ベクトル
        double determinant;                 // 共分散行列の行列式
    };
    std::vector<Ellipse> previousEllipses_;  // アンカーごとの前回の楕円を保持
    //パーティクルの初期配置
    void init_particles(int n) {
    // 各軸方向のグリッド数を計算
        int grid_size = std::cbrt(particle_count_); // 一辺の粒子数
        if (grid_size * grid_size * grid_size < particle_count_) {
            ++grid_size; // 粒子数が完全に埋まらない場合、一辺のサイズを増やす
        }
        // グリッド間隔を計算
        double x_y_range_min = -15.0;
        double x_y_range_max = 15.0;
        double z_range_min = 0.0;
        double z_range_max = 5.0;
        double x_y_step = (x_y_range_max - x_y_range_min) / (grid_size - 1); // x, y軸の等間隔
        double z_step = (z_range_max - z_range_min) / (grid_size - 1);       // z軸の等間隔
        // パーティクル群を保持するベクターをリサイズ
        particle_groups_.resize(n);
        for (int group_idx = 0; group_idx < n; ++group_idx) {
         // 各パーティクル群のリサイズ
            particle_groups_[group_idx].resize(particle_count_);
         // グリッド上に等間隔に粒子を配置
            int index = 0;
            for (int i = 0; i < grid_size && index < particle_count_; ++i) {
                for (int j = 0; j < grid_size && index < particle_count_; ++j) {
                    for (int k = 0; k < grid_size && index < particle_count_; ++k) {
                        // 各粒子の位置を設定
                        double x = x_y_range_min + i * x_y_step;
                        double y = x_y_range_min + j * x_y_step;
                        double z = z_range_min + k * z_step;

                        particle_groups_[group_idx][index].position.setValue(x, y, z);
                        particle_groups_[group_idx][index].weight = 1.0 / particle_count_; // 初期重みを均等に設定
                        ++index;
                    }
                }
            }
        }
    }
    void save_robot_position(const tf2::Vector3& position) {
        robot_positions_.push_back(position);
    }
    // round(linspace(1, t, 5)) 相当の処理をする関数
    void process_robot_positions() {
        size_t t = robot_positions_.size();
        if (t >= 5) {
            // linspace equivalent with round
            std::vector<int> indices;
            double step = static_cast<double>(t - 1) / 4;  // 5つの位置を計算
            for (int i = 0; i < 5; ++i) {
                int idx = static_cast<int>(round(i * step)); // round(linspace(1, t, 5))
                indices.push_back(idx);
            }           
            // 選ばれた位置を出力
            for (size_t i = 0; i < indices.size(); ++i) {
                int idx = indices[i];
                Robot_anchor_positions[i] = robot_positions_[idx];  // Robot_anchor_positionsに格納
            }
        } else {
            // ロボットの位置が5個未満の場合、0番目の位置を5回出力
            for (int i = 0; i < 5; ++i) {
                if (t > 0) {
                    Robot_anchor_positions[i] = robot_positions_[0];  // Robot_anchor_positionsに格納
                }
            }
        }
    }
    bool is_valid_double(const std::string& str)
    {
        // 数字や小数点を含んでいるかチェック
        std::stringstream ss(str);
        double d;
        return (ss >> d) && (ss.eof());  // 正常にdoubleとして変換できればtrue
    }
    void topic1_callback(const std::shared_ptr<std_msgs::msg::String> msg)
    {
        received[0] = true;
        position = msg->data;
        position.erase(std::remove(position.begin(), position.end(), '['), position.end());
        position.erase(std::remove(position.begin(), position.end(), ']'), position.end());
        std::vector<double> values;
        std::stringstream ss(position);
        std::string value;
        while (std::getline(ss, value, ',')) {
            if (is_valid_double(value)) {
                values.push_back(std::stod(value));  // 文字列をdoubleに変換してベクトルに追加
            } else {
                RCLCPP_WARN(this->get_logger(), "Invalid value found: %s", value.c_str());
            }
        }
        if (values.size() == 16) {
        // 4x4 行列の形式で表示
            // RCLCPP_INFO(this->get_logger(), "[%.6f, %.6f, %.6f]",values[12], values[13], values[14]);
        } else {
            RCLCPP_WARN(this->get_logger(), "Received unexpected number of values: %zu", values.size());
        }
        Robot_position.setValue(values[12], values[13], values[14]);
        save_robot_position(Robot_position);  // ロボット位置を保存
        process_robot_positions();  // ロボット位置に基づいて選択された位置を処理
        calculate_distance();
        publish_robot_marker();
    }

   void topic2_callback(const std::shared_ptr<geometry_msgs::msg::PoseStamped>msg)
    {
        Anchor_positions[0].setValue(msg->pose.position.x, msg->pose.position.y, msg->pose.position.z);
        received[1] = true;
        calculate_distance();
    }
    void topic3_callback(const std::shared_ptr<geometry_msgs::msg::PoseStamped>msg)
    {
        Anchor_positions[1].setValue(msg->pose.position.x, msg->pose.position.y, msg->pose.position.z);
        received[2] = true;
        calculate_distance();
    }
   void topic4_callback(const std::shared_ptr<geometry_msgs::msg::PoseStamped>msg)
    {
        Anchor_positions[2].setValue(msg->pose.position.x, msg->pose.position.y, msg->pose.position.z);
        received[3] = true;
        calculate_distance();
    }
   void topic5_callback(const std::shared_ptr<geometry_msgs::msg::PoseStamped>msg)
    {
        Anchor_positions[3].setValue(msg->pose.position.x, msg->pose.position.y, msg->pose.position.z);
        received[4] = true;
        calculate_distance();
    }
   void topic6_callback(const std::shared_ptr<geometry_msgs::msg::PoseStamped>msg)
    {
        Anchor_positions[4].setValue(msg->pose.position.x, msg->pose.position.y, msg->pose.position.z);
        received[5] = true;
        calculate_distance();
        publish_anchor4_marker();
    }
    void imu_callback5(const custom_message::msg::ImuMsg::SharedPtr msg) {
        // IMUデータの処理
        auto first_string = msg->first_string.data;
        auto second_string = msg->second_string.data;

        // RCLCPP_INFO(this->get_logger(), "Raw First String: '%s'", first_string.c_str());
        // RCLCPP_INFO(this->get_logger(), "Raw Second String: '%s'", second_string.c_str());

        // データのパース
        std::vector<double> angular_velocity = parseVectorFromString(first_string);
        std::vector<double> acceleration = parseVectorFromString(second_string);

        // データの長さチェック
        if (angular_velocity.size() != 3 || acceleration.size() != 3) {
            std::cerr << "Error: Unexpected vector size. Using default values." << std::endl;
            imu_gyro = Vector3d(0.0, 0.0, 0.0);
            imu_acc = Vector3d(0.0, 0.0, 9.81); // デフォルト値
        } else {
            imu_gyro = Vector3d(angular_velocity[0], angular_velocity[1], angular_velocity[2]);
            imu_acc = Vector3d(acceleration[0], acceleration[1], acceleration[2]);
            
        }
        // 現在のタイムスタンプ
        rclcpp::Time current_time = msg->header.stamp;
         // 最初のコールバックの場合、last_timeを設定しない
        if (last_time.nanoseconds() == 0) {
            last_time = current_time;
            return;  // 最初のループではdtを計算しない
        }
        // 前回のタイムスタンプと現在のタイムスタンプとの差を計算（秒単位）
        double dt = (current_time - last_time).seconds();
        
        // dtをログ出力（必要に応じて）
        RCLCPP_INFO(this->get_logger(), "Time difference (dt): %.3f seconds", dt);

        // 前回のタイムスタンプを更新
        last_time = current_time;
        imu_dt = dt;

        // 加速度ベクトルの大きさを計算
        double acceleration_magnitude = std::sqrt(
            std::pow(acceleration[0], 2) + 
            std::pow(acceleration[1], 2) + 
            std::pow(acceleration[2], 2)
        );

        // 移動判定
        double reference_magnitude = 9.81;  // 静止時の基準値
        double threshold = 1.0;             // 判定のしきい値

        if (std::abs(acceleration_magnitude - reference_magnitude) >= threshold) {
            anchor_move[4] = 1;  // 移動中
            // RCLCPP_INFO(this->get_logger(), "Movement Detected No.3: 1");
        } else {
            anchor_move[4] = 0;  // 静止
            // RCLCPP_INFO(this->get_logger(), "Movement Detected No.3: 0");
        }

        // データのログ出力
        RCLCPP_INFO(this->get_logger(), "Angular Velocity: [%.2f, %.2f, %.2f]",
                    angular_velocity[0], angular_velocity[1], angular_velocity[2]);
        RCLCPP_INFO(this->get_logger(), "Acceleration: [%.2f, %.2f, %.2f] (Magnitude: %.2f)",
                    acceleration[0], acceleration[1], acceleration[2], acceleration_magnitude);

        received[6] = true;
        calculate_distance();
    }
    // 文字列から数値リストを抽出する関数
    std::vector<double> parseVectorFromString(const std::string& data) {
        std::vector<double> result;
        std::stringstream ss(data.substr(1, data.size() - 2));  // '['と']'を除去
        std::string item;

        while (std::getline(ss, item, ',')) {
            try {
                result.push_back(std::stod(item));  // 文字列をdoubleに変換
            } catch (const std::exception& e) {
                RCLCPP_WARN(this->get_logger(), "Failed to parse: [%s]", item.c_str());
            }
        }
        return result;
    }
        // 距離計算とパーティクル更新
    void calculate_distance() {
        if (received[0]&&received[1]&&received[2]&&received[3]&&received[4]&&received[5]&&received[6]) {
            static bool is_initialized = false;
            timer+=1;
            RCLCPP_INFO(this->get_logger(),"Time is %.d",timer);
            if (!is_initialized) {
                x.segment<3>(0) = Eigen::Vector3d(0.0, 0.0, 4.75);  // 初期位置
                x.segment<3>(3) = Eigen::Vector3d(0.0, 0.0, 0.0);  // 初期速度
                x.segment<4>(6) = Eigen::Vector4d(1.0, 0.0, 0.0, 0.0);  // 初期クォータニオン
                is_initialized = true;
            }

            //RCLCPP_INFO(this->get_logger(), "Time difference (dt): %.3f seconds", imu_dt);
            Eigen::Vector3d displacement_p = displace_cube(imu_dt);
            RCLCPP_INFO(this->get_logger(), "Displacement(x: %f, y: %f, z: %f)",
                    displacement_p.x(), displacement_p.y(), displacement_p.z());
            std::fill(std::begin(received), std::end(received), false);
        }
    }
    Eigen::Vector3d displace_cube(double dt){
         // 状態遷移
        Eigen::VectorXd x_next;
        state_transition(x,imu_acc, imu_gyro, dt, x_next);

        //RCLCPP_INFO(this->get_logger(), "Time difference (dt): %.3f seconds", dt);
        // 更新された位置を使用
        Eigen::Vector3d corrected_position = x_next.segment<3>(0);

        geometry_msgs::msg::Point p;
        p.x = corrected_position.x();
        p.y = corrected_position.y();
        p.z = corrected_position.z();
        RCLCPP_INFO(this->get_logger(), "Position(x: %f, y: %f, z: %f)",
                        corrected_position.x(), corrected_position.y(), corrected_position.z());
        // 推定位置をパブリッシュ (PoseStamped)
        geometry_msgs::msg::PoseStamped estimated_pose_msg;
        estimated_pose_msg.header.stamp = this->get_clock()->now();
        estimated_pose_msg.header.frame_id = "map";  // RVizで使用する座標系
        estimated_pose_msg.pose.position.x = corrected_position.x();
        estimated_pose_msg.pose.position.y = corrected_position.y();
        estimated_pose_msg.pose.position.z = corrected_position.z();
        estimated_pose_msg.pose.orientation.w = 1.0;  // 向きが不要であればデフォルト値
        estimated_pose_publisher_->publish(estimated_pose_msg);

        // 推定位置をパブリッシュ (Marker)
        visualization_msgs::msg::Marker marker_msg;
        marker_msg.header.stamp = this->get_clock()->now();
        marker_msg.header.frame_id = "map";  // RVizの座標系
        marker_msg.ns = "corrected_position";
        marker_msg.id = marker_id_counter_++;  // 一意のIDを設定
        marker_msg.type = visualization_msgs::msg::Marker::SPHERE;
        marker_msg.action = visualization_msgs::msg::Marker::ADD;

        marker_msg.pose.position.x = corrected_position.x();
        marker_msg.pose.position.y = corrected_position.y();
        marker_msg.pose.position.z = corrected_position.z();
        marker_msg.pose.orientation.w = 1.0;

        marker_msg.scale.x = 0.1;  // マーカーのサイズ
        marker_msg.scale.y = 0.1;
        marker_msg.scale.z = 0.1;

        marker_msg.color.r = 0.0;  // 赤色
        marker_msg.color.g = 0.0;
        marker_msg.color.b = 1.0;
        marker_msg.color.a = 1.0;  // 不透明度
        marker_msg.lifetime = rclcpp::Duration::from_seconds(0.0);

        marker_publishers_[anchor_idx]->publish(marker_msg);

        marker_history_.push(marker_msg.id);
        if (marker_history_.size() > max_markers_) {
        // 古いマーカーを削除
            visualization_msgs::msg::Marker delete_msg;
            delete_msg.header.stamp = this->get_clock()->now();
            delete_msg.header.frame_id = "map";
            delete_msg.ns = "estimated_position";
            delete_msg.id = marker_history_.front();  // 最古のIDを取得
            delete_msg.action = visualization_msgs::msg::Marker::DELETE;
            marker_publishers_[anchor_idx]->publish(delete_msg);
            marker_history_.pop();  // 履歴から削除
        }
        x=x_next;
        return corrected_position;
    }
       // 推定位置を計算
    void estimate_position(size_t anchor_idx,double dt) {
       
    }
    // クォータニオンの微分を計算する関数
    Eigen::Vector4d computeQuaternionDerivative(const Eigen::Vector4d &q, const Eigen::Vector3d &omega) {
        double qw = q(0), qx = q(1), qy = q(2), qz = q(3);
        double wx = omega(0), wy = omega(1), wz = omega(2);

        Eigen::Vector4d q_dot;
        q_dot << -qx * wx - qy * wy - qz * wz,
                qw * wx - qz * wy + qy * wz,
                qz * wx + qw * wy - qx * wz,
                -qy * wx + qx * wy + qw * wz;
        q_dot *= 0.5;

        return q_dot;
    }


   // 状態遷移関数
    void state_transition(
        Eigen::VectorXd &x,
        const Vector3d &acc,
        const Vector3d &gyro,
        double dt,
        Eigen::VectorXd &x_next) 
    {
        // 状態ベクトルから各成分を抽出
        Vector3d pos = x.segment<3>(0);  // 位置 (累積されるべき)
        Vector3d vel = x.segment<3>(3);  // 速度
        Eigen::Vector4d q = x.segment<4>(6); // クォータニオン

        // クォータニオンの更新
        Eigen::Vector4d q_dot = computeQuaternionDerivative(q, gyro);
        Eigen::Vector4d q_4_next = q + q_dot * dt;
        q_4_next.normalize(); // クォータニオンを正規化
        Quaterniond q_next(q_4_next(0), q_4_next(1), q_4_next(2), q_4_next(3));

        // 加速度から重力の影響を除去
        Vector3d g(0.0, 0.0, 9.81);  // 重力ベクトル（m/s^2）
        Vector3d acc_corrected = acc-g ;
        //RCLCPP_WARN(this->get_logger(), "[%.2f, %.2f, %.2f] ",acc_corrected[0], acc_corrected[1], acc_corrected[2]);

        // 加速度をローカル座標系からグローバル座標系に変換
        //Vector3d acc_global = q_next.conjugate() * acc_corrected;

        // 速度と位置の更新
        Vector3d vel_next = vel + acc_corrected * dt;
        //RCLCPP_WARN(this->get_logger(), "[%.2f, %.2f, %.10f] ",vel_next[0], vel_next[1], vel_next[2]);

        Vector3d displacement =vel * dt + 0.5 * acc_corrected * dt * dt;
        //RCLCPP_WARN(this->get_logger(), "[%.2f, %.2f, %.2f] ",vel_next[0], vel_next[1], vel_next[2]);
        Vector3d pos_next = pos + displacement; // 累積的に位置を更新

        RCLCPP_INFO(this->get_logger(), "displacement(x: %f, y: %f, z: %f)",
                        displacement[0], displacement[1], displacement[2]);

        // 次の状態ベクトルを形成
        x_next.resize(10);
        x_next.segment<3>(0) = pos_next;
        x_next.segment<3>(3) = vel_next;
        x_next.segment<4>(6) = q_4_next;
    }
    void publish_robot_marker() {
        visualization_msgs::msg::Marker robot_marker;
        robot_marker.header.stamp = this->get_clock()->now();
        robot_marker.header.frame_id = "map";  // 座標系の指定
        robot_marker.ns = "robot_position";    // 名前空間を分離
        robot_marker.id = robot_marker_id_counter_++;                   // 固定ID（更新するため）
        robot_marker.type = visualization_msgs::msg::Marker::SPHERE;
        robot_marker.action = visualization_msgs::msg::Marker::ADD;

        robot_marker.pose.position.x = Robot_position.x();
        robot_marker.pose.position.y = Robot_position.y();
        robot_marker.pose.position.z = Robot_position.z();
        robot_marker.pose.orientation.w = 1.0;

        robot_marker.scale.x = 0.2;  // Markerサイズ
        robot_marker.scale.y = 0.2;
        robot_marker.scale.z = 0.2;

        robot_marker.color.r = 1.0;  // 赤色
        robot_marker.color.g = 0.0;
        robot_marker.color.b = 0.0;
        robot_marker.color.a = 1.0;  // 不透明

        robot_marker.lifetime = rclcpp::Duration::from_seconds(0.0);  // 永続表示
        robot_marker_publisher_->publish(robot_marker);
        if (robot_marker_history_.size() > robot_max_markers_) {
        // 古いマーカーを削除
            visualization_msgs::msg::Marker delete_msg;
            delete_msg.header.stamp = this->get_clock()->now();
            delete_msg.header.frame_id = "map";
            delete_msg.ns = "robot_position";
            delete_msg.id = robot_marker_history_.front();  // 最古のIDを取得
            delete_msg.action = visualization_msgs::msg::Marker::DELETE;
            robot_marker_publisher_->publish(delete_msg);
            robot_marker_history_.pop();  // 履歴から削除
        }
    }
    void publish_anchor4_marker() {
        visualization_msgs::msg::Marker anchor4_marker;
        anchor4_marker.header.stamp = this->get_clock()->now();
        anchor4_marker.header.frame_id = "map";  // 座標系の指定
        anchor4_marker.ns = "anchor_position";    // 名前空間を分離
        anchor4_marker.id = robot_marker_id_counter_++;                   // 固定ID（更新するため）
        anchor4_marker.type = visualization_msgs::msg::Marker::SPHERE;
        anchor4_marker.action = visualization_msgs::msg::Marker::ADD;

        anchor4_marker.pose.position.x = Anchor_positions[4].x();
        anchor4_marker.pose.position.y = Anchor_positions[4].y();
        anchor4_marker.pose.position.z = Anchor_positions[4].z();
        anchor4_marker.pose.orientation.w = 1.0;

        anchor4_marker.scale.x = 0.2;  // Markerサイズ
        anchor4_marker.scale.y = 0.2;
        anchor4_marker.scale.z = 0.2;

        anchor4_marker.color.r = 1.0;  // 赤色
        anchor4_marker.color.g = 1.0;
        anchor4_marker.color.b = 1.0;
        anchor4_marker.color.a = 1.0;  // 不透明

        anchor4_marker.lifetime = rclcpp::Duration::from_seconds(0.0);  // 永続表示
        anchor4_marker_publisher_->publish(anchor4_marker);
        if (anchor4_marker_history_.size() > anchor4_max_markers_) {
        // 古いマーカーを削除
            visualization_msgs::msg::Marker delete_msg;
            delete_msg.header.stamp = this->get_clock()->now();
            delete_msg.header.frame_id = "map";
            delete_msg.ns = "anchor4_position";
            delete_msg.id = anchor4_marker_history_.front();  // 最古のIDを取得
            delete_msg.action = visualization_msgs::msg::Marker::DELETE;
            anchor4_marker_publisher_->publish(delete_msg);
            anchor4_marker_history_.pop();  // 履歴から削除
        }
    }
    //確認用
    void display_particles(size_t anchor_idx) {
        RCLCPP_INFO(this->get_logger(), "Displaying particles for Anchor %ld", anchor_idx);

        for (size_t i = 0; i < particle_groups_[anchor_idx].size(); ++i) {
            const Particle& particle = particle_groups_[anchor_idx][i];
            RCLCPP_INFO(this->get_logger(), "Particle %ld: Position(x: %f, y: %f, z: %f), Weight: %f",
                        i, particle.position.x(), particle.position.y(), particle.position.z(), particle.weight);
        }
    }
};


int main(int argc,char * argv[])
{
    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<calc>());
    rclcpp::shutdown();
    return 0;
}