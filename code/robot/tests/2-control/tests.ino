/*
    ...
*/

    /*
        Perturbation
        Determine max angle of perturbation
        Full speed obstacle avoidance
    */

//============
void test(byte test_id) {
    switch (test_id) {
        case 1: test_1(); break;
        case 2: test_2(); break;
        case 3: test_3(); break;
        case 4: test_4(); break;
        case 5: test_5(); break;
        case 6: test_6(); break;
        case 7: test_7(); break;
        case 8: test_8(); break;
        case 9: test_9(); break;
        case 10: test_10(); break;
        case 11: test_11(); break;
        case 12: test_12(); break;
    }
}



//============
void test_1() {
    /*
        Test bidirectional communication.
    */

    int delay_ = 5;
    FrequencyState* freq_msg = new FrequencyState(5);
    FrequencyState* freq_obstacle = new FrequencyState(5);

    while (true) {
        if (freq_msg->isNewState()) {
            receive_msg_pid();
            Serial.println("Kp of speed pid: " + String(pid_speed->getKp()));
        }
        if (freq_obstacle->isNewState() && sensors->isObstacle()) break;
        
        delay(delay_);   
    }
    actuators->stop();
}


//============
void test_2() {
    /*
        Test random data to visualize on realtime subplots.
    */

    int delay_ = 50;
    while (true) {
        Serial.println("5/5.5/6/4");
        delay(delay_);
        if (sensors->isObstacle()) break;
    }
}


//============
void test_3() {
    /*
        Tune PID speed motor on left motor. Target speed varies as a wave
        function. (robot is in an elevated position)

        Best tuning so far:
        Kp = 20 
        Ki = 0.012
        f = 5
    */

    pid_speed->setParameters(20, 0, 0.012);
    pid_speed->reset();
    sensors->encodersReset();

    int delay_ = 5;
    FrequencyState* freq_receiver = new FrequencyState(10);
    FrequencyState* freq_sender = new FrequencyState(20);
    FrequencyState* freq_obstacle = new FrequencyState(5);
    FrequencyState* freq_wave = new FrequencyState(0.1);
    FrequencyState* freq_speed_control = new FrequencyState(10);
    FrequencyState* freq_acceleration = new FrequencyState(20);

    int pwm = 0;
    float progress_speed = 0, target_speed = 6, measured_speed = 0;
    float alpha = 0, beta = 0;
    
    Accelerator* acc = new Accelerator(0.15);
    acc->start(progress_speed, target_speed, 1.5);
    for (int seq_nb = 0; true; delay(delay_)) {
        if (freq_receiver->isNewState()) receive_msg_pid();   
        if (freq_obstacle->isNewState() && sensors->isObstacle()) break;
        if (freq_speed_control->isNewState()) {
            sensors->encodersRead();
            beta = speed_control_left(progress_speed);
        }
        if (freq_wave->isNewState() && acc->saturation(progress_speed)) {
            if (target_speed == 6) target_speed = 8;
            else target_speed = 6;
            acc->start(progress_speed, target_speed, 3);
        }
        if (freq_sender->isNewState()) {
            measured_speed = sensors->getSpeedLeft();
            Serial.println(String(seq_nb) + "/" + String(progress_speed) + "/" + String(measured_speed));
            seq_nb++;
        }
        pwm = beta;
        actuators->updatePWM(0, pwm);
        if (freq_acceleration->isNewState()) acc->accelerate(progress_speed);
    }
    actuators->stop();
}


//============
void test_4() {
    /*
        Tune PID speed control and forward control together. 
        (robot surélevé)

        Best tuning:
        Kp_speed = 12
        Ki_speed = 0.022
        f_speed = 5
        Kp_forward = 15
        Kd_forward = 0.07
        Ki_forward = 0.065
        f_forward = 25
    */

    pid_speed->setParameters(12, 0, 0.022);
    //pid_forward->setParameters(15, 0.07, 0.065);
    pid_forward->setParameters(0, 0, 0);
    pid_speed->reset();
    pid_forward->reset();
    sensors->encodersReset();

    int delay_ = 5;
    FrequencyState* freq_receiver = new FrequencyState(10);
    FrequencyState* freq_sender = new FrequencyState(20);
    FrequencyState* freq_obstacle = new FrequencyState(10);
    FrequencyState* freq_wave = new FrequencyState(0.1);
    FrequencyState* freq_speed_control = new FrequencyState(10);
    FrequencyState* freq_direction_control = new FrequencyState(50);
    FrequencyState* freq_acceleration = new FrequencyState(20);

    float alpha = 0, beta = 0, progress_speed = 0, target_speed = 6;
    int pwm_left = 0, pwm_right = 0;

    Accelerator* acc = new Accelerator(0.25);
    acc->start(progress_speed,target_speed, 1.5);
    for (int seq_nb = 0; true; delay(delay_)) {
        if (freq_receiver->isNewState()) receive_msg_pid();
        if (freq_obstacle->isNewState() && sensors->isObstacle()) break;
        if (freq_direction_control->isNewState()) {
            sensors->encodersRead();
            alpha = forward_control();
            if (freq_speed_control->isNewState()) 
                beta = speed_control(progress_speed);
        }

        pwm_left = beta + alpha;
        pwm_right = beta - alpha;
        actuators->updatePWM(pwm_left, pwm_right);
        if (freq_acceleration->isNewState()) acc->accelerate(progress_speed);

        if (freq_wave->isNewState()) {
            if (target_speed == 6) target_speed = 8;
            else target_speed = 6;
            acc->start(progress_speed, target_speed, 3);
        }

        if (freq_sender->isNewState()) {
            Serial.println(String(seq_nb) + "/" + String(progress_speed) + "/" +\
                String(sensors->getSpeed()) + "/" + String(sensors->getSpeedLeft())\
                + "/" + String(sensors->getSpeedRight()));
            seq_nb++;
        }
    }
    actuators->stop();
}



//============
void test_5() {
    /*
        Forward (straight line) at constant speed and stops at obstacle.
    */

    pid_speed->setParameters(12, 0, 0.022);
    pid_forward->setParameters(15, 0.08, 0.065);

    pid_speed->reset();
    pid_forward->reset();
    sensors->encodersReset();

    int delay_ = 5;
    FrequencyState* freq_obstacle = new FrequencyState(10);
    FrequencyState* freq_speed_control = new FrequencyState(10);
    FrequencyState* freq_direction_control = new FrequencyState(50);
    FrequencyState* freq_acceleration = new FrequencyState(20);

    float alpha = 0, beta = 0, progress_speed = 2;
    int pwm_left = 0, pwm_right = 0;

    Accelerator* acc = new Accelerator(0.2);
    acc->start(progress_speed, 6, 2.5);
    for (int seq_nb = 0; true; delay(delay_)) {
        if (freq_obstacle->isNewState() && sensors->isObstacle()) break;
        if (freq_direction_control->isNewState()) {
            sensors->encodersRead();
            alpha = forward_control();
            if (freq_speed_control->isNewState()) 
                beta = speed_control(progress_speed);
        }
        pwm_left = beta + alpha;
        pwm_right = beta - alpha;
        actuators->updatePWM(pwm_left, pwm_right);
        if (freq_acceleration->isNewState()) acc->accelerate(progress_speed);
    }
    actuators->stop();
}



//============
void test_6() {
    /*
        Follows a complex curved line at constant speed and stops at obstacle with continuation.

        Kp = 0.025, Kd = 0.0008, Ki = 0.00008 (f = 50) if scale 2500
        60,0,0.2 (f=50, scale 2 0.01)
        50,0.025,0.15(f=50,scale 2 0.01 zeta=3.1415/2)
        
    */

    flicker_led(led_signal, 10, 300);
    digitalWrite(led_signal, HIGH); 
    sensors->manualCalibration();
    digitalWrite(led_signal, LOW); 

    pid_speed->reset();
    pid_line->reset();
    pid_line->setZeta(1000);
    sensors->encodersReset();

    pid_speed->setParameters(12,0,0.022);
    pid_line->setParameters(0,0,0);

    int delay_ = 5;
    FrequencyState* freq_receiver = new FrequencyState(10);
    FrequencyState* freq_obstacle = new FrequencyState(10);
    FrequencyState* freq_speed_control = new FrequencyState(20);
    FrequencyState* freq_direction_control = new FrequencyState(50);
    FrequencyState* freq_acceleration = new FrequencyState(20);

    float alpha = 0, beta = 0, progress_speed = 0;
    int pwm_left = 0, pwm_right = 0;

    Accelerator* acc = new Accelerator(0.1);
    instruction = -1;
    bool wait = false;
    for (;true;delay(delay_)) {
        
        if (freq_obstacle->isNewState() && sensors->isObstacle()) {
            acc->stop(progress_speed);
            actuators->stop();
            pid_speed->reset();
            pid_line->reset();
            delay(10000);
            wait = true;
        }
        if (freq_receiver->isNewState() && !wait) receive_msg_line();
        if (instruction != 1 && !wait) {
            acc->stop(progress_speed);
            actuators->stop();
            pid_speed->reset();
            pid_line->reset();
            continue;
        }
        if (instruction == 1 && !acc->isRunning() && !wait) { 
            pid_line->reset();
            acc->start(progress_speed, 7.5, 0.8);
        }
        if (freq_direction_control->isNewState() && !wait) {
            sensors->qtraRead();
            alpha = line_control();
            if (freq_speed_control->isNewState()) {
                sensors->encodersRead();
                beta = speed_control(progress_speed);
            }
            pwm_left = beta + alpha;
            pwm_right = beta - alpha;
            actuators->updatePWM(pwm_left, pwm_right);
        }
        if (acc->isRunning() && freq_acceleration->isNewState() && !wait) acc->accelerate(progress_speed);
        wait = false;
    }
    actuators->stop();
}


//============
void test_7() {
    /*
        Turn at constant speed (robot in elevated position)
    */

    pid_speed->setParameters(12, 0, 0.022);
    pid_forward->setParameters(15, 0.07, 0.065);
    
    pid_speed->reset();
    pid_forward->reset();
    sensors->encodersReset();

    int delay_ = 5;
    FrequencyState* freq_receiver = new FrequencyState(10);
    FrequencyState* freq_sender = new FrequencyState(20);
    FrequencyState* freq_obstacle = new FrequencyState(10);
    FrequencyState* freq_wave = new FrequencyState(0.01);
    FrequencyState* freq_speed_control = new FrequencyState(5);
    FrequencyState* freq_direction_control = new FrequencyState(25);
    FrequencyState* freq_acceleration = new FrequencyState(5);

    float alpha = 0, beta = 0, progress_speed = 0, target_speed = 6;
    int pwm_left = 0, pwm_right = 0;

    Accelerator* acc = new Accelerator(0.1);
    acc->start(progress_speed, 6, 2);
    for (int seq_nb = 0; true; delay(delay_)) {
        if (freq_receiver->isNewState()) receive_msg_pid();
        if (freq_obstacle->isNewState() && sensors->isObstacle()) break;
        if (freq_direction_control->isNewState()) {
            sensors->encodersRead();
            alpha = forward_control();
            if (freq_speed_control->isNewState()) 
                beta = speed_control(progress_speed);
        }

        pwm_left = beta + alpha;
        pwm_right = -(beta - alpha);
        actuators->updatePWM(pwm_left, pwm_right);
        if (freq_acceleration->isNewState()) acc->accelerate(progress_speed);

        if (freq_wave->isNewState()) {
            if (target_speed == 6) target_speed = 8;
            else target_speed = 6;
            acc->start(progress_speed, target_speed, 3);
        }

        if (freq_sender->isNewState()) {
            Serial.println(String(seq_nb) + "/" + String(progress_speed) + "/" +\
                String(sensors->getSpeed()) + "/" + String(sensors->getSpeedLeft())\
                + "/" + String(sensors->getSpeedRight()));
            seq_nb++;
        }
    }
    actuators->stop();
}


//============
void test_8() {
    /*
        Control turn at constant speed 
    */

    pid_speed->setParameters(12, 0, 0.022);
    pid_forward->setParameters(15, 0.07, 0.065);
    
    pid_speed->reset();
    pid_forward->reset();
    sensors->encodersReset();

    int delay_ = 5;
    FrequencyState* freq_receiver = new FrequencyState(10);
    FrequencyState* freq_obstacle = new FrequencyState(10);
    FrequencyState* freq_speed_control = new FrequencyState(5);
    FrequencyState* freq_direction_control = new FrequencyState(25);
    FrequencyState* freq_acceleration = new FrequencyState(5);

    float alpha = 0, beta = 0, progress_speed = 0, target_speed = 6;
    int pwm_left = 0, pwm_right = 0;

    Accelerator* acc = new Accelerator(0.1);
    acc->start(progress_speed, 6, 2);
    for (int seq_nb = 0; true; delay(delay_)) {
        if (freq_receiver->isNewState()) receive_msg_pid();
        if (freq_obstacle->isNewState() && sensors->isObstacle()) break;
        if (freq_direction_control->isNewState()) {
            sensors->encodersRead();
            alpha = forward_control();
            if (freq_speed_control->isNewState()) 
                beta = speed_control(progress_speed);
        }

        pwm_left = beta + alpha;
        pwm_right = -(beta - alpha);
        actuators->updatePWM(pwm_left, pwm_right);
        if (freq_acceleration->isNewState()) acc->accelerate(progress_speed);
    }
    actuators->stop();
}

//============
void test_9() {
    /*
        Alternating between 90-180 degrees in both directions. 
    */
    
    pid_speed->setParameters(12, 0, 0.022);
    pid_forward->setParameters(15, 0.07, 0.065);
    
    pid_speed->reset();
    pid_forward->reset();
    sensors->encodersReset();

    int delay_ = 5;
    FrequencyState* freq_receiver = new FrequencyState(10);
    FrequencyState* freq_obstacle = new FrequencyState(10);
    FrequencyState* freq_speed_control = new FrequencyState(10);
    FrequencyState* freq_angular_check = new FrequencyState(20);
    FrequencyState* freq_direction_control = new FrequencyState(50);
    FrequencyState* freq_acceleration = new FrequencyState(10);

    float alpha = 0, beta = 0, progress_speed = 3, target_speed = 6;
    int pwm_left = 0, pwm_right = 0;

    Accelerator* acc = new Accelerator(0.05);
    bool clock_wise = true;
    float theta_goal = 3.1415/4;
    float current_theta;
    unsigned long prev_t = millis();
    unsigned long current_t = prev_t;
    for (int i = 0; true; i++) {
        if (freq_obstacle->isNewState() && sensors->isObstacle()) break;
        acc->start(progress_speed, 6, 1);
        unsigned long start_t = millis();
        clock_wise = !clock_wise;
        current_theta = 0;
        if (i % 2 == 0) theta_goal = (theta_goal == 3.1415/2) ? 3.1415/4 : 3.1415/2;
        for (int i = 0; true; delay(delay_)) {
            if (freq_receiver->isNewState()) receive_msg_pid();
            if (freq_obstacle->isNewState() && sensors->isObstacle()) break;
            if (freq_direction_control->isNewState()) {
                sensors->encodersRead();
                alpha = forward_control();
                if (freq_speed_control->isNewState()) 
                    beta = speed_control(progress_speed);
            }
            pwm_left = beta + alpha;
            pwm_right = beta - alpha;
            if (!clock_wise) actuators->updatePWM(pwm_left, -pwm_right);
            else actuators->updatePWM(-pwm_left, pwm_right);
            if (freq_acceleration->isNewState()) acc->accelerate(progress_speed);
            if (freq_angular_check->isNewState()) {
                current_t = millis();
                float delta_t = (float) (current_t-prev_t);
                current_theta += 2*sensors->getSpeed()*delta_t/1000/9.3;
                if (current_theta >= theta_goal) break;
                Serial.println(current_theta);
                prev_t = current_t;
            }
        }
        acc->stop(progress_speed);
        actuators->stop();
        delay(200);
        pid_speed->reset();
        pid_forward->reset();
    }
    
}


//============
void test_10() {
    /* Little back and forth movements with calibration sensors at the same time */
    // BUGS TODO

    pid_speed->setParameters(12, 0, 0.022);
    pid_forward->setParameters(15, 0.07, 0.065);

    pid_speed->reset();
    pid_forward->reset();
    sensors->encodersReset();

    int delay_ = 5;
    FrequencyState* freq_obstacle = new FrequencyState(10);
    FrequencyState* freq_speed_control = new FrequencyState(10);
    FrequencyState* freq_direction_control = new FrequencyState(50);

    float alpha = 0, beta = 0, progress_speed = 5;
    int pwm_left = 0, pwm_right = 0;
    bool forward = true;
    unsigned long init_t = millis();
    int i = 0;
    while (millis() > (init_t + 14000)) {
    // while (i < 100) {
        if (freq_obstacle->isNewState() && sensors->isObstacle()) break;
        unsigned long start_t = millis();
        forward = !forward;
        while (true) {
            if (millis() > (start_t + 500)) break; 
            if (freq_obstacle->isNewState() && sensors->isObstacle()) break;
            if (freq_direction_control->isNewState()) {
                sensors->encodersRead();
                alpha = forward_control();
                if (freq_speed_control->isNewState()) 
                    beta = speed_control(progress_speed);
            }
            pwm_left = beta + alpha;
            pwm_right = beta - alpha;
            if (forward) actuators->updatePWM(pwm_left, pwm_right);
            else actuators->updatePWM(-pwm_left, -pwm_right);
            delay(5);
        }
        actuators->stop();
        // for (int j = 0; j < 50; j++) {
        //     sensors->oneStepCalibrate();
        //     i++;
        // }
        // Serial.println(i);
        pid_speed->reset();
        pid_forward->reset();
    }
    
}


//============
void test_11() {
    /* 
        Perturbation plot: follow line with different initial angle perturbation.
        Send error line at high frequency to estimate oscillation on line.
    */

    flicker_led(led_signal, 10, 300);
    digitalWrite(led_signal, HIGH); 
    sensors->manualCalibration();
    digitalWrite(led_signal, LOW); 
    delay(5000);

    pid_speed->reset();
    pid_line->reset();
    sensors->encodersReset();
    pid_speed->setParameters(12,0,0.022);
    pid_line->setZeta(3.1415/2.5);
    pid_line->setParameters(50,0.025,0.15);

    int delay_ = 5;
    FrequencyState* freq_sending = new FrequencyState(3);
    FrequencyState* freq_receiver = new FrequencyState(10);
    FrequencyState* freq_obstacle = new FrequencyState(10);
    FrequencyState* freq_speed_control = new FrequencyState(20);
    FrequencyState* freq_direction_control = new FrequencyState(50);
    FrequencyState* freq_acceleration = new FrequencyState(20);

    float alpha = 0, beta = 0, progress_speed = 0;
    int pwm_left = 0, pwm_right = 0;

    int test_number = 4;
    unsigned long start_t = millis();
    sensors->qtraRead();
    messenger->sendMessage("test_number;t;err_line");
    messenger->sendMessage(String(test_number)+";"+String(0)+";"+String(sensors->getError()));
    delay(1000);

    Accelerator* acc = new Accelerator(0.1);
    acc->start(progress_speed, 6, 1.5);
    pid_line->reset(); // Zeta init t
    for (;true;delay(delay_)) {
        if (freq_obstacle->isNewState() && sensors->isObstacle()) {
            actuators->stop();
            break;
        }
        if (freq_direction_control->isNewState()) {
            sensors->qtraRead();
            alpha = line_control();
            if (freq_speed_control->isNewState()) {
                sensors->encodersRead();
                beta = speed_control(progress_speed);
            }
            if (freq_sending->isNewState()) {
                float t = ((float) millis()-start_t)/1000.;
                messenger->sendMessage(String(test_number)+";"+String(t)+";"+String(sensors->getError()));
            }
            pwm_left = beta + alpha;
            pwm_right = beta - alpha;
            actuators->updatePWM(pwm_left, pwm_right);
        }
        if (acc->isRunning() && freq_acceleration->isNewState()) acc->accelerate(progress_speed);
    }
    actuators->stop();
}


//============
void test_12() {
    ;
}
