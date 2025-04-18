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
#include <fstream>
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

        imu_subscriber1_ = this->create_subscription<custom_message::msg::ImuMsg>(
        "IMU_data1", 10, std::bind(&calc::imu_callback1, this, std::placeholders::_1));

        imu_subscriber2_ = this->create_subscription<custom_message::msg::ImuMsg>(
        "IMU_data2", 10, std::bind(&calc::imu_callback2, this, std::placeholders::_1));

        imu_subscriber3_ = this->create_subscription<custom_message::msg::ImuMsg>(
        "IMU_data3", 10, std::bind(&calc::imu_callback3, this, std::placeholders::_1));

        imu_subscriber4_ = this->create_subscription<custom_message::msg::ImuMsg>(
        "IMU_data4", 10, std::bind(&calc::imu_callback4, this, std::placeholders::_1));

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
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr displace_marker_publisher_;
    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr particle_marker_publishers_[5];
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr time_publisher_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr anchor_error_publishers_[5];  // アンカーごとの誤差をパブリッシュ
    rclcpp::Subscription<custom_message::msg::ImuMsg>::SharedPtr imu_subscriber1_;
    rclcpp::Subscription<custom_message::msg::ImuMsg>::SharedPtr imu_subscriber2_;
    rclcpp::Subscription<custom_message::msg::ImuMsg>::SharedPtr imu_subscriber3_;
    rclcpp::Subscription<custom_message::msg::ImuMsg>::SharedPtr imu_subscriber4_;
    rclcpp::Subscription<custom_message::msg::ImuMsg>::SharedPtr imu_subscriber5_;
    int gizi=1;//疑似アンカーありなら1なしなら0
    Eigen::VectorXd x;  // 状態ベクトル
    int timer=0;
    rclcpp::Time last_time;
    double imu_dt; 
    tf2::Vector3 Anchor_positions[5];  // 5つのアンカーの位置 (事前に設定された位置)
    tf2::Vector3 displacement_p;
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
    const int displace_max_markers_ = 100;      // 同時に表示する最大マーカー数
    double Robot[3];
    int anchor_move[5];
    int anchor_estimation_complete[5];
    int available_anchor[5];
    int is_resampling_started[5]={0,0,0,0,0};
    int time[5]={0,0,0,0,0};
    double max_weight[5];
    bool received[7] = { false, false,false, false, false, false,false};
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
    double dt=0.03;
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
    void imu_callback1(const custom_message::msg::ImuMsg::SharedPtr msg) {
        // IMUデータの処理
        auto first_string = msg->first_string.data;
        auto second_string = msg->second_string.data;

        // RCLCPP_INFO(this->get_logger(), "Raw First String: '%s'", first_string.c_str());
        // RCLCPP_INFO(this->get_logger(), "Raw Second String: '%s'", second_string.c_str());

        // データのパース
        std::vector<double> angular_velocity = parseVectorFromString(first_string);
        std::vector<double> acceleration = parseVectorFromString(second_string);

        // データの長さチェック
        if (acceleration.size() != 3) {
            RCLCPP_WARN(this->get_logger(), "Unexpected vector size for acceleration. Using default values.");
            acceleration = {0.0, 0.0, 9.81};  // デフォルト値
        }

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
            anchor_move[0] = 1;  // 移動中
            // RCLCPP_INFO(this->get_logger(), "Movement Detected No.1: 1");
        } else {
            anchor_move[0] = 0;  // 静止
            // RCLCPP_INFO(this->get_logger(), "Movement Detected No.1: 0");
        }

        // データのログ出力
        // RCLCPP_INFO(this->get_logger(), "Parsed Angular Velocity: [%.2f, %.2f, %.2f]",
        //             angular_velocity[0], angular_velocity[1], angular_velocity[2]);
        // RCLCPP_INFO(this->get_logger(), "Parsed Acceleration: [%.2f, %.2f, %.2f] (Magnitude: %.2f)",
        //             acceleration[0], acceleration[1], acceleration[2], acceleration_magnitude);
    }

    void imu_callback2(const custom_message::msg::ImuMsg::SharedPtr msg) {
        // IMUデータの処理
        auto first_string = msg->first_string.data;
        auto second_string = msg->second_string.data;

        // RCLCPP_INFO(this->get_logger(), "Raw First String: '%s'", first_string.c_str());
        // RCLCPP_INFO(this->get_logger(), "Raw Second String: '%s'", second_string.c_str());

        // データのパース
        std::vector<double> angular_velocity = parseVectorFromString(first_string);
        std::vector<double> acceleration = parseVectorFromString(second_string);

        // データの長さチェック
        if (acceleration.size() != 3) {
            RCLCPP_WARN(this->get_logger(), "Unexpected vector size for acceleration. Using default values.");
            acceleration = {0.0, 0.0, 9.81};  // デフォルト値
        }

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
            anchor_move[1] = 1;  // 移動中
            // RCLCPP_INFO(this->get_logger(), "Movement Detected No.2: 1");
        } else {
            anchor_move[1] = 0;  // 静止
            // RCLCPP_INFO(this->get_logger(), "Movement Detected No.2: 0");
        }

        // データのログ出力
        // RCLCPP_INFO(this->get_logger(), "Parsed Angular Velocity: [%.2f, %.2f, %.2f]",
        //             angular_velocity[0], angular_velocity[1], angular_velocity[2]);
        // RCLCPP_INFO(this->get_logger(), "Parsed Acceleration: [%.2f, %.2f, %.2f] (Magnitude: %.2f)",
        //             acceleration[0], acceleration[1], acceleration[2], acceleration_magnitude);
    }
    void imu_callback3(const custom_message::msg::ImuMsg::SharedPtr msg) {
        // IMUデータの処理
        auto first_string = msg->first_string.data;
        auto second_string = msg->second_string.data;

        // RCLCPP_INFO(this->get_logger(), "Raw First String: '%s'", first_string.c_str());
        // RCLCPP_INFO(this->get_logger(), "Raw Second String: '%s'", second_string.c_str());

        // データのパース
        std::vector<double> angular_velocity = parseVectorFromString(first_string);
        std::vector<double> acceleration = parseVectorFromString(second_string);

        // データの長さチェック
        if (acceleration.size() != 3) {
            RCLCPP_WARN(this->get_logger(), "Unexpected vector size for acceleration. Using default values.");
            acceleration = {0.0, 0.0, 9.81};  // デフォルト値
        }

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
            anchor_move[2] = 1;  // 移動中
            // RCLCPP_INFO(this->get_logger(), "Movement Detected No.3: 1");
        } else {
            anchor_move[2] = 0;  // 静止
            // RCLCPP_INFO(this->get_logger(), "Movement Detected No.3: 0");
        }

        // データのログ出力
        // RCLCPP_INFO(this->get_logger(), "Parsed Angular Velocity: [%.2f, %.2f, %.2f]",
        //             angular_velocity[0], angular_velocity[1], angular_velocity[2]);
        // RCLCPP_INFO(this->get_logger(), "Parsed Acceleration: [%.2f, %.2f, %.2f] (Magnitude: %.2f)",
        //             acceleration[0], acceleration[1], acceleration[2], acceleration_magnitude);
    }
        void imu_callback4(const custom_message::msg::ImuMsg::SharedPtr msg) {
        // IMUデータの処理
        auto first_string = msg->first_string.data;
        auto second_string = msg->second_string.data;

        // RCLCPP_INFO(this->get_logger(), "Raw First String: '%s'", first_string.c_str());
        // RCLCPP_INFO(this->get_logger(), "Raw Second String: '%s'", second_string.c_str());

        // データのパース
        std::vector<double> angular_velocity = parseVectorFromString(first_string);
        std::vector<double> acceleration = parseVectorFromString(second_string);

        // データの長さチェック
        if (acceleration.size() != 3) {
            RCLCPP_WARN(this->get_logger(), "Unexpected vector size for acceleration. Using default values.");
            acceleration = {0.0, 0.0, 9.81};  // デフォルト値
        }

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
            anchor_move[3] = 1;// 移動中
            // RCLCPP_INFO(this->get_logger(), "Movement Detected No.4: 1");
        } else {
            anchor_move[3] = 0;  // 静止
            // RCLCPP_INFO(this->get_logger(), "Movement Detected No.4: 0");
        }

        // データのログ出力
        // RCLCPP_INFO(this->get_logger(), "Parsed Angular Velocity: [%.2f, %.2f, %.2f]",
        //             angular_velocity[0], angular_velocity[1], angular_velocity[2]);
        // RCLCPP_INFO(this->get_logger(), "Parsed Acceleration: [%.2f, %.2f, %.2f] (Magnitude: %.2f)",
        //             acceleration[0], acceleration[1], acceleration[2], acceleration_magnitude);
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
        //RCLCPP_INFO(this->get_logger(), "Angular Velocity: [%.2f, %.2f, %.2f]",
        //            angular_velocity[0], angular_velocity[1], angular_velocity[2]);
        //RCLCPP_INFO(this->get_logger(), "Acceleration: [%.2f, %.2f, %.2f] (Magnitude: %.2f)",
        //            acceleration[0], acceleration[1], acceleration[2], acceleration_magnitude);
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
        static std::vector<tf2::Vector3> anchor_positions_at_6300(5);
        static std::array<int, 5> available_anchor_at_6300 = {0};
        if (received[0]&&received[1]&&received[2]&&received[3]&&received[4]&&received[5]&&received[6]) {
            static bool is_initialized = false;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::normal_distribution<> noise_dist(0.0, 0.1);
            std::uniform_real_distribution<> uniform_dist(0.0, 1.0);  
            std::array<double, 5> noise_values;
            for (size_t i = 0; i < 5; ++i) {
                noise_values[i] = uniform_dist(gen);
            }
            double resampling_threshold_=0.7; // リサンプリングを行う最大重みのしきい値
            double sigma =0.1;
            static std::vector<Ellipse> previousEllipses(5);
            timer+=1;
                    // tをトピックとしてパブリッシュ
            auto msg = std_msgs::msg::Int32();
            msg.data = timer;
            time_publisher_->publish(msg);
            RCLCPP_INFO(this->get_logger(),"Time is %.d",timer);
            if (timer == 4000) {
                for (size_t i = 0; i < 5; ++i) {
                    anchor_positions_at_6300[i] = Anchor_positions[i];  // 位置情報を保存
                    available_anchor_at_6300[i] = available_anchor[i];  // available_anchorも保存
                }
            }
            for (size_t anchor_idx = 0; anchor_idx < 5; ++anchor_idx) {
                check_estimation_and_movement(anchor_idx);
            }
            Eigen::VectorXd x(10);
            if (!is_initialized) {
                x.segment<3>(0) = Eigen::Vector3d(0.0, 0.0, 0.0);  // 初期位置
                x.segment<3>(3) = Eigen::Vector3d(0.0, 0.0, 0.0);  // 初期速度
                x.segment<4>(6) = Eigen::Vector4d(1.0, 0.0, 0.0, 0.0);  // 初期クォータニオン
                is_initialized = true;
            }
            // 状態遷移を実行し、移動量を計算
            Eigen::Vector3d displacement_p = displace_cube(imu_dt);
            
            for (size_t anchor_idx = 0; anchor_idx < 5; ++anchor_idx) {
                time[anchor_idx]=time[anchor_idx]+1;
                if (available_anchor[anchor_idx] == 1&&time[anchor_idx]>500) {
                    noise_dist = std::normal_distribution<>(0.0, 0.01);  // 推定済みかつ移動していない場合のノイズ
                    sigma = 0.15;
                }else if (is_resampling_started[anchor_idx] == 1) {
                    noise_dist = std::normal_distribution<>(0.0, 0.03);  // リサンプリング後のノイズ
                    sigma = 0.3;
                }else{
                    noise_dist = std::normal_distribution<>(0.0, 0.1); 
                    sigma =1;
                }
                for (auto &p : particle_groups_[anchor_idx]) {
                    if (is_resampling_started[anchor_idx]==1) {
                        // ノイズを追加
                        p.position.setX(p.position.x() + noise_dist(gen));
                        p.position.setY(p.position.y() + noise_dist(gen));
                        p.position.setZ(p.position.z() + noise_dist(gen));
                        
                        if (anchor_idx==4) {
                            p.position.setX(p.position.x()+displacement_p[0]);
                            p.position.setY(p.position.y()+displacement_p[1]);
                            p.position.setZ(p.position.z()+displacement_p[2]);
                        }
                    }
                }
                const double sigma_squared =sigma*sigma ; 
                const double sqrt_2_pi_sigma = std::sqrt(2 * M_PI * sigma_squared); 
                for (auto &p : particle_groups_[anchor_idx]) {
                    double likelihood = 1;
                    // ロボットの位置を使用
                    if(timer<4000){
                        for (size_t i = 0; i < 5; ++i) {
                            double expected_distance = tf2::tf2Distance(p.position, Robot_anchor_positions[i]);
                            double actual_distance = tf2::tf2Distance(Anchor_positions[anchor_idx],Robot_anchor_positions[i]);
                            actual_distance =actual_distance+0.05*noise_values[i];
                            // 尤度の計算
                            double diff = expected_distance - actual_distance;
                            double likelihood_component = (1.0 / sqrt_2_pi_sigma) * std::exp(-(diff * diff) / (2 * sigma_squared));
                            likelihood *= likelihood_component;
                        }
                    }else{
                        double expected_distance = tf2::tf2Distance(p.position, Robot_anchor_positions[4]);
                        double actual_distance = tf2::tf2Distance(Anchor_positions[anchor_idx],Robot_anchor_positions[4]);
                        actual_distance =actual_distance+0.05* noise_values[4];
                        // 尤度の計算
                        double diff = expected_distance - actual_distance;
                        double likelihood_component = (1.0 / sqrt_2_pi_sigma) * std::exp(-(diff * diff) / (2 * sigma_squared));
                        likelihood *= likelihood_component;
                    
                    }
                    p.weight *= likelihood;
                    // Z軸が負の場合、重みを0.5倍
                    if (p.position.z() < 0.0) {
                        p.weight *= 0.5;
                    }
                    if(timer>1&&gizi==1&&anchor_idx<4){//移動前のタグ1~4を推定
                        // 推定済みかつ動いていないアンカーを追加で使用
                        for (size_t i = 0; i < 5; ++i) {
                            if (available_anchor[i] == 1 && anchor_idx!=i) {  // 利用可能なアンカーのみを使用
                                tf2::Vector3 static_anchor_position = estimated_positions_history[i].back();  // 最新の推定位置を利用

                                double expected_distance = tf2::tf2Distance(p.position, static_anchor_position);
                                double actual_distance = tf2::tf2Distance(Anchor_positions[anchor_idx], static_anchor_position);

                                // 尤度の計算
                                double diff = expected_distance - actual_distance;
                                double likelihood_component = (1.0 / sqrt_2_pi_sigma) * std::exp(-(diff * diff) / (2 * sigma_squared));
                                likelihood *= likelihood_component;
                            }
                        }
                        p.weight *= likelihood;
                        // Z軸が負の場合、重みを0.5倍
                        if (p.position.z() < 0.0) {
                            p.weight *= 0.5;
                        }
                    }//
                    if(timer>1&&timer<=4000&&gizi==1&&anchor_idx==4){//移動前のタグ5のみ推定
                        // 推定済みかつ動いていないアンカーを追加で使用
                        for (size_t i = 0; i < 5; ++i) {
                            if (available_anchor[i] == 1 && anchor_idx!=i) {  // 利用可能なアンカーのみを使用
                                tf2::Vector3 static_anchor_position = estimated_positions_history[i].back();  // 最新の推定位置を利用

                                double expected_distance = tf2::tf2Distance(p.position, static_anchor_position);
                                double actual_distance = tf2::tf2Distance(Anchor_positions[anchor_idx], static_anchor_position);

                                // 尤度の計算
                                double diff = expected_distance - actual_distance;
                                double likelihood_component = (1.0 / sqrt_2_pi_sigma) * std::exp(-(diff * diff) / (2 * sigma_squared));
                                likelihood *= likelihood_component;
                            }
                        }
                        p.weight *= likelihood;
                        // Z軸が負の場合、重みを0.5倍
                        if (p.position.z() < 0.0) {
                            p.weight *= 0.5;
                        }
                    }
                    if(timer>4000&&gizi==1&&anchor_idx==4){//移動後のタグ5のみ推定
                        // 推定済みかつ動いていないアンカーを追加で使用
                        for (size_t i = 0; i < 5; ++i) {
                            if ( anchor_idx!=i&&available_anchor_at_6300[i] == 1) {  // 利用可能なアンカーのみを使用
                                double expected_distance = tf2::tf2Distance(p.position, anchor_positions_at_6300[i]);
                                double actual_distance = tf2::tf2Distance(Anchor_positions[anchor_idx], anchor_positions_at_6300[i]);

                                // 尤度の計算
                                double diff = expected_distance - actual_distance;
                                double likelihood_component = (1.0 / sqrt_2_pi_sigma) * std::exp(-(diff * diff) / (2 * sigma_squared));
                                likelihood *= likelihood_component;
                            }
                        }
                        p.weight *= likelihood;
                        // Z軸が負の場合、重みを0.5倍
                        if (p.position.z() < 0.0) {
                            p.weight *= 0.5;
                        }
                    }
                }
                normalize_weights(anchor_idx);
                estimate_position(anchor_idx);
                //確認用
                //if(anchor_idx==2){
                //    display_particles(2);
                //}
                max_weight[anchor_idx] = (*std::max_element(
                                        particle_groups_[anchor_idx].begin(), particle_groups_[anchor_idx].end(),
                                        [](const Particle& a, const Particle& b) { return a.weight < b.weight; }))
                                        .weight;
                // 粒子群の位置情報を収集
                std::vector<Eigen::Vector3d> particle_positions;
                for (const auto& p : particle_groups_[anchor_idx]) {
                    particle_positions.emplace_back(p.position.x(), p.position.y(), p.position.z());
                }
                // 確率楕円を計算
                previousEllipses_[anchor_idx] = errorEllipse(particle_positions);
                // 最大の重みがしきい値を超える場合のみリサンプリングを実行
                if (max_weight[anchor_idx] >= resampling_threshold_||is_resampling_started[anchor_idx] == 1) {
                    is_resampling_started[anchor_idx] = 1; // リサンプリング開始フラグを立てる
                    resample_particles(anchor_idx);
                    
                }
                if(timer<4000){
                    publish_particle_markers(anchor_idx);
                }
                if(timer>=4000&&anchor_idx==4){
                    publish_particle_markers(anchor_idx);
                }
            }
            std::fill(std::begin(received), std::end(received), false);
        }
    }
        // 重みの正規化
    void normalize_weights(size_t anchor_idx) {
        double sum_weights = std::accumulate(particle_groups_[anchor_idx].begin(), particle_groups_[anchor_idx].end(), 0.0,
                                             [](double sum, const Particle& p) { return sum + p.weight; });
        if (sum_weights!=0){
            for (auto& p : particle_groups_[anchor_idx]) {
                p.weight /= sum_weights;
            }
        }else{
            for (auto& p : particle_groups_[anchor_idx]) {
                p.weight = 1.0 / particle_count_;
            }
        }
        // RCLCPP_INFO(this->get_logger(),"%.4f",sum_weights);
    }
       // 推定位置を計算
    void estimate_position(size_t anchor_idx) {
        static std::ofstream error_log("error_log.csv", std::ios::app);
        static bool header_written = false;
        if (particle_groups_[anchor_idx].empty()) {
            RCLCPP_WARN(this->get_logger(), "No particles available for estimation");
            return;
        }

        // 重みが最大のパーティクルを探す
        auto max_weight_particle = std::max_element(
            particle_groups_[anchor_idx].begin(), particle_groups_[anchor_idx].end(),
            [](const Particle& a, const Particle& b) {
                return a.weight < b.weight;
            });

        // 最大の重みを持つパーティクルの位置を推定値とする
        tf2::Vector3 estimated_position = max_weight_particle->position;
        double pw=max_weight_particle->weight;

        // 推定値をログに出力
        RCLCPP_INFO(this->get_logger(), "Estimated Position (Max Weight) at Anchor %zu: x=%.2f, y=%.2f, z=%.2f, weight=%.6f",
                    anchor_idx,estimated_position.x(), estimated_position.y(), estimated_position.z(),pw);
        // 推定履歴のチェックと更新
        if (estimated_positions_history[anchor_idx].size() < max_history_size) {
            // 履歴がまだ50個に満たない場合
            estimated_positions_history[anchor_idx].push_back(estimated_position);
            // RCLCPP_INFO(this->get_logger(),"Anchor %zu: Estimation Incomplete (0) - Insufficient History", anchor_idx);
        } else {
            // 履歴が50個以上ある場合、古いデータを削除して新しいデータを追加
            estimated_positions_history[anchor_idx].pop_front();
            estimated_positions_history[anchor_idx].push_back(estimated_position);

            // x, y, z 軸ごとに標準偏差を計算
            double mean_x = 0.0, mean_y = 0.0, mean_z = 0.0;
            size_t num_positions = estimated_positions_history[anchor_idx].size();

            // 平均値を計算
            for (const auto& pos : estimated_positions_history[anchor_idx]) {
                mean_x += pos.x();
                mean_y += pos.y();
                mean_z += pos.z();
            }
            mean_x /= num_positions;
            mean_y /= num_positions;
            mean_z /= num_positions;

            // 標準偏差を計算
            double variance_x = 0.0, variance_y = 0.0, variance_z = 0.0;
            for (const auto& pos : estimated_positions_history[anchor_idx]) {
                variance_x += std::pow(pos.x() - mean_x, 2);
                variance_y += std::pow(pos.y() - mean_y, 2);
                variance_z += std::pow(pos.z() - mean_z, 2);
            }
            double stddev_x = std::sqrt(variance_x / num_positions);
            double stddev_y = std::sqrt(variance_y / num_positions);
            double stddev_z = std::sqrt(variance_z / num_positions);

            // 標準偏差の合計を求める
            double total_stddev = stddev_x + stddev_y + stddev_z;
            // 推定が完了しているか判定
            double threshold = 0.1;  // 完了とみなす距離の閾値
            if (total_stddev <= threshold) {
                anchor_estimation_complete[anchor_idx] = 1;
                // RCLCPP_INFO(this->get_logger(),"Anchor %zu: Estimation Complete (1)", anchor_idx);
            } else {
                anchor_estimation_complete[anchor_idx] = 0;
                // RCLCPP_INFO(this->get_logger(),"Anchor %zu: Estimation Incomplete (0)", anchor_idx);
            }

            // 最大距離をログ出力
            RCLCPP_INFO(this->get_logger(), 
                        "Anchor %zu: Max Distance in Last50 Positions: %.3f", 
                        anchor_idx, total_stddev);
        }
         // 実際のロボット位置との誤差を計算
        tf2::Vector3 error_vector =  Anchor_positions[anchor_idx]-estimated_position ;
        double total_error = error_vector.length(); // ユークリッド距離
        // 前回の誤差を取得
        double previous_error = previous_errors_[anchor_idx];
        // 誤差をログに出力
        RCLCPP_INFO(this->get_logger(),
                    "Error at Anchor %zu: x=%.3f, y=%.3f, z=%.3f, Total Error=%.3f",
                    anchor_idx,error_vector.x(), error_vector.y(), error_vector.z(), total_error);
        RCLCPP_INFO(this->get_logger(),"Total Error Past=%.3f",previous_errors_[anchor_idx]);
                // 誤差をパブリッシュ
        std_msgs::msg::Float64 error_msg;
        if(timer>=4000&&anchor_idx<4){
            total_error=previous_errors_[anchor_idx];
        }
        RCLCPP_INFO(this->get_logger(),
                    "Error at Anchor %zu: total Error=%.3f",
                    anchor_idx,total_error);
        error_msg.data = total_error;
        anchor_error_publishers_[anchor_idx]->publish(error_msg);
        // 現在の誤差を前回の誤差として保存
        previous_errors_[anchor_idx] = total_error;
        // CSVに書き込む処理
        if (!header_written) {
            error_log << "Anchor_0,Anchor_1,Anchor_2,Anchor_3,Anchor_4\n";
            header_written = true;
        }
        static std::vector<double> error_values(5, 0.0);
        error_values[anchor_idx] = total_error;

        if (anchor_idx == 4) {  // すべてのアンカーのデータが揃ったら出力
            for (size_t i = 0; i < 5; ++i) {
                error_log << error_values[i];
                if (i < error_values.size() - 1) error_log << ",";
            }
            error_log << "\n";
            error_log.flush();  // データを即時書き込み
        }
        error_msg.data = total_error;
        anchor_error_publishers_[anchor_idx]->publish(error_msg);
        // 推定位置をパブリッシュ (PoseStamped)
        geometry_msgs::msg::PoseStamped estimated_pose_msg;
        estimated_pose_msg.header.stamp = this->get_clock()->now();
        estimated_pose_msg.header.frame_id = "map";  // RVizで使用する座標系
        estimated_pose_msg.pose.position.x = estimated_position.x();
        estimated_pose_msg.pose.position.y = estimated_position.y();
        estimated_pose_msg.pose.position.z = estimated_position.z();
        estimated_pose_msg.pose.orientation.w = 1.0;  // 向きが不要であればデフォルト値
        estimated_pose_publisher_->publish(estimated_pose_msg);

        // 推定位置をパブリッシュ (Marker)
        visualization_msgs::msg::Marker marker_msg;
        marker_msg.header.stamp = this->get_clock()->now();
        marker_msg.header.frame_id = "map";  // RVizの座標系
        marker_msg.ns = "estimated_position";
        marker_msg.id = marker_id_counter_++;  // 一意のIDを設定
        marker_msg.type = visualization_msgs::msg::Marker::SPHERE;
        marker_msg.action = visualization_msgs::msg::Marker::ADD;

        marker_msg.pose.position.x = estimated_position.x();
        marker_msg.pose.position.y = estimated_position.y();
        marker_msg.pose.position.z = estimated_position.z();
        marker_msg.pose.orientation.w = 1.0;

        marker_msg.scale.x = 0.1;  // マーカーのサイズ
        marker_msg.scale.y = 0.1;
        marker_msg.scale.z = 0.1;

        marker_msg.color.r = 0.0;  // 赤色
        marker_msg.color.g = 0.0;
        marker_msg.color.b = 1.0;
        if(timer>=4000&&anchor_idx<4){
            marker_msg.color.a = 0.0;  // 不透明度
        }else{
            marker_msg.color.a = 1.0;  // 不透明度
        }
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
    }
    // リサンプリング
    void resample_particles(size_t anchor_idx) {
        std::vector<Particle> new_particles;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dist(0.0, 1.0);

    // 最大重みを求める
        double max_weight_re = (*std::max_element(particle_groups_[anchor_idx].begin(), particle_groups_[anchor_idx].end(),
                                               [](const Particle& a, const Particle& b) { return a.weight < b.weight; }))
                                .weight;
        if (max_weight_re == 0.0) {
            for (auto& p : particle_groups_[anchor_idx]) {
                p.weight = 1.0 / particle_count_; // 重みをリセット
            }
            max_weight_re = 1.0 / particle_count_; // デフォルト値を設定
        }
    // ランダムに初期の位置を決定
        double step = 1.0 / particle_count_;
        double beta = dist(gen) * step;
        int index = 0;
        
        double cumulative_weight = particle_groups_[anchor_idx][index].weight;
        for (size_t i = 0; i < particle_count_; ++i) {
        // β が現在のパーティクルの重みを超えると次のパーティクルを選ぶ
            while (beta > cumulative_weight) {
                index = (index + 1) % particle_count_;
                cumulative_weight += particle_groups_[anchor_idx][index].weight;
            }
        // パーティクルを新しいリストに追加
            new_particles.push_back(particle_groups_[anchor_idx][index]);
            beta +=step;  // 次の β を増加
        }
        particle_groups_[anchor_idx] = new_particles;
        // 重みを均等に設定
        double uniform_weight = 1.0 / particle_count_;
        for (auto& p : particle_groups_[anchor_idx]) {
            p.weight = uniform_weight;
        }
    }
    void check_estimation_and_movement(size_t anchor_idx) {
        // 推定済みかつ移動していないかを判定
        if (anchor_estimation_complete[anchor_idx] == 1 && anchor_move[anchor_idx] == 0) {
            RCLCPP_INFO(this->get_logger(), "Anchor %zu: Estimation Complete and No Movement!!!!!!", anchor_idx);
            available_anchor[anchor_idx]=1;
        } else if (anchor_estimation_complete[anchor_idx] == 1 && anchor_move[anchor_idx] == 1) {
            RCLCPP_INFO(this->get_logger(), "Anchor %zu: Estimation Complete but Moving", anchor_idx);
            available_anchor[anchor_idx]=0;
        } else if (anchor_estimation_complete[anchor_idx] == 0 && anchor_move[anchor_idx] == 0) {
            RCLCPP_INFO(this->get_logger(), "Anchor %zu: Estimation Not Complete", anchor_idx);
            available_anchor[anchor_idx]=0;
        }
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
        Vector3d pos = x.segment<3>(0);  // 位置
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

        Vector3d displacement =vel * dt + 0.5 * acc_corrected * dt * dt;
        Vector3d pos_next = pos + displacement; // 累積的に位置を更新

        // 次の状態ベクトルを形成
        x_next.resize(10);
        x_next.segment<3>(0) = displacement;
        x_next.segment<3>(3) = vel_next;
        x_next.segment<4>(6) = q_4_next;
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
        x=x_next;
        return corrected_position;
    }

    // 確率楕円を計算する関数
    Ellipse errorEllipse(const std::vector<Eigen::Vector3d>& points) {
        if (points.empty()) {
            throw std::invalid_argument("Points vector is empty.");
        }

        // 点群の平均（楕円の中心）
        Eigen::Vector3d center = Eigen::Vector3d::Zero();
        for (const auto& point : points) {
            center += point;
        }
        center /= points.size();

        // 共分散行列を計算
        Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
        for (const auto& point : points) {
            Eigen::Vector3d diff = point - center;
            covariance += diff * diff.transpose();
        }
        covariance /= points.size();

        // 固有値と固有ベクトルを計算
        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(covariance);
        Eigen::Vector3d eigVal = solver.eigenvalues();
        Eigen::Matrix3d eigVec = solver.eigenvectors();

        // 固有値を降順に並べ替え
        std::vector<std::pair<double, Eigen::Vector3d>> eigenPairs;
        for (int i = 0; i < 3; ++i) {
            eigenPairs.emplace_back(eigVal[i], eigVec.col(i));
        }
        std::sort(eigenPairs.begin(), eigenPairs.end(),
                [](const std::pair<double, Eigen::Vector3d>& a, const std::pair<double, Eigen::Vector3d>& b) {
                    return a.first > b.first;
                });

        for (int i = 0; i < 3; ++i) {
            eigVal[i] = eigenPairs[i].first;
            eigVec.col(i) = eigenPairs[i].second;
        }

        // 95%信頼区間用スケールファクタ
        double chiSquareValue = std::sqrt(5.991);
        Eigen::Vector3d side = chiSquareValue * eigVal.cwiseSqrt();

        // 固有値を基に計算した行列式
        double determinant = eigVal[0] * 100 * eigVal[1] * 100 * eigVal[2] * 100;

        // 結果を格納
        Ellipse ellipse;
        ellipse.center = center;
        ellipse.side = side;
        ellipse.eigVec = eigVec;
        ellipse.determinant = determinant;
        return ellipse;
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
    void publish_particle_markers(size_t anchor_idx) {
        // マーカー配列メッセージを作成
        visualization_msgs::msg::MarkerArray marker_array;

        // パーティクルごとにマーカーを生成
        int marker_id = 0;
        for (const auto& particle : particle_groups_[anchor_idx]) {
            visualization_msgs::msg::Marker marker;
            marker.header.frame_id = "map"; // RVizで使用するフレームID
            marker.header.stamp = this->get_clock()->now();
            marker.ns = "particles";
            marker.id = marker_id++;
            marker.type = visualization_msgs::msg::Marker::SPHERE; // 球形マーカー
            marker.action = visualization_msgs::msg::Marker::ADD;
            marker.pose.position.x = particle.position.x();
            marker.pose.position.y = particle.position.y();
            marker.pose.position.z = particle.position.z();
            marker.pose.orientation.w = 1.0;
            marker.scale.x = 0.2; // パーティクルの大きさ
            marker.scale.y = 0.2;
            marker.scale.z = 0.2;
            marker.color.r = 0.0; // マーカーの色
            marker.color.g = 1.0;
            marker.color.b = 0.0;
            marker.color.a = 1.0; // 不透明度
            // マーカーを配列に追加
            marker_array.markers.push_back(marker);
        }

        // 配列をパブリッシュ
        particle_marker_publishers_[anchor_idx]->publish(marker_array);
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
    void log_errors_to_csv() {
        std::ofstream csv_file;
        std::string filename = "/home/robotdeveloper02/IsaacSim-ros_workspaces-main/humble_ws/error_log.csv";

        // ファイルが存在するか確認
        bool file_exists = std::ifstream(filename).good();

        // 追記モードで開く
        csv_file.open(filename, std::ios::app);
        if (!csv_file) {
            RCLCPP_ERROR(this->get_logger(), "Failed to open CSV file: %s", filename.c_str());
            return;
        }

        // ヘッダー行（最初の書き込み時のみ）
        if (!file_exists) {
            csv_file << "Timestamp";
            for (size_t i = 0; i < 5; ++i) {
                csv_file << ",Anchor_" << i;
            }
            csv_file << "\n";
        }

        // タイムスタンプ
        auto now = this->get_clock()->now();
        csv_file << now.seconds();

        // 各アンカーの total_error を出力
        for (size_t i = 0; i < 5; ++i) {
            csv_file << "," << previous_errors_[i];  // 最新の誤差を出力
        }
        csv_file << "\n";

        csv_file.close();
        RCLCPP_INFO(this->get_logger(), "Total errors saved to CSV.");
    }
};


int main(int argc,char * argv[])
{
    rclcpp::init(argc,argv);
    rclcpp::spin(std::make_shared<calc>());
    rclcpp::shutdown();
    return 0;
}