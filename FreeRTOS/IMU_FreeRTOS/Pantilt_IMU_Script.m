%Script IMU_FreeRTOS
clear all;
close all;

%Define Serial Ports:
IMU_Port = 'COM3';
PanTilt_Port = 'COM5';

%Configure constants for test:
tTest       = 10; %segundos
tTaskSample = 20; %ms
QAngle      = 0.001;
QBias       = 0.003;
RMeasure    = 0.03;
alpha       = 0.98; %0.7143;

%Initialize Variables for Sample:
sample = (tTest*1000)/tTaskSample;
acel = zeros(sample,3);
gyro = zeros(sample,3);
angle = zeros(sample,3);
encoder = zeros(sample,1);

%Variable for PanTilt:
Tp = 2;
Ts = tTaskSample/1000;
Iteration = tTest/Tp;
max_angle = 30;
[tilt_steps, angle_steps, Tempo] = SineTiltGenerate(max_angle,Ts,Tp,Iteration);

%OPEN SERIAL PORTS:
s_IMU = serial(IMU_Port,'BaudRate', 115200, 'DataBits', 8, 'StopBits', 1, 'Parity', 'none', 'Timeout', 3, 'Terminator', 'CR/LF');
fclose(s_IMU);
fopen(s_IMU);
s_PAN = serial(PanTilt_Port,'BaudRate', 9600, 'DataBits', 8, 'StopBits', 1, 'Parity', 'none', 'Timeout', 3, 'Terminator', 'CR/LF');
fclose(s_PAN);
fopen(s_PAN);
disp('Serial Ports Opened');

%Parse command strings for IMU:
str_tTest       = sprintf('tTotalSample;1;%.3f',tTest);
str_tTaskSample = sprintf('tTaskSample;1;%.3f',tTaskSample);
str_QAngle      = sprintf('kalQAngle;1;%.3f',QAngle);
str_QBias       = sprintf('kalQBias;1;%.3f',QBias);
str_RMeasure    = sprintf('kalRMeasure;1;%.3f',RMeasure);
str_Alpha       = sprintf('alphaCFilter;1;%.3f',alpha);
%Parse command strings for PAN:
str_Tiltpos     = sprintf('TP%.0f',tilt_steps(1));

%Send commands:
disp('Sending Configuration Commands');
fprintf(s_IMU,'%s\r',str_tTest);
fscanf(s_IMU);
fprintf(s_IMU,'%s\r',str_tTaskSample);
fscanf(s_IMU);
fprintf(s_IMU,'%s\r',str_QAngle);
fscanf(s_IMU);
fprintf(s_IMU,'%s\r',str_QBias);
fscanf(s_IMU);
fprintf(s_IMU,'%s\r',str_RMeasure);
fscanf(s_IMU);
fprintf(s_IMU,'%s\r',str_Alpha);
fscanf(s_IMU);

%Send initial position:
fprintf(s_PAN,'%s\r',str_Tiltpos);
fscanf(s_PAN);
fprintf(s_IMU,'%s\r','resetVar');
fscanf(s_IMU);
pause(2);   %Time to IMU initialize its variables again

%Start Command:
fprintf(s_IMU,'%s\r','goReset');
disp('IMU Read Started!');

for i = 1:sample
    %READ IMU DATA
    out = fscanf(s_IMU);
    strVal = strsplit(out, ';');
    
    k = 1;
    acel(i,1) = str2double(strVal(k));
    k= k+1;
    acel(i,2) = str2double(strVal(k));
    k= k+1;
    acel(i,3) = str2double(strVal(k));
    k= k+1;
    
    gyro(i,1) = str2double(strVal(k));
    k= k+1;
    gyro(i,2) = str2double(strVal(k));
    k= k+1;
    gyro(i,3) = str2double(strVal(k));
    k= k+1;
    
    angle(i,1) = str2double(strVal(k));
    k= k+1;
    angle(i,2) = str2double(strVal(k));
    k= k+1;
    angle(i,3) = str2double(strVal(k));
    
    k= k+1;
    encoder(i) = str2double(strVal(k));
    
    %UPDATE PANTILT POSITION:
    str_Tiltpos = sprintf('TP%.0f',tilt_steps(i));
    fprintf(s_PAN,'%s\r',str_Tiltpos);
    fscanf(s_PAN);
end

fclose(s_IMU);
delete(s_IMU);
fclose(s_PAN);
delete(s_PAN);

disp('Serial Finished');

figure;
plot(Tempo, angle(:,1));
hold on;
plot(Tempo, angle(:,2), 'r');
hold on;
plot(Tempo, angle(:,3), 'k');
hold on;
plot(Tempo, angle_steps, 'm');
hold on;
legend('Pure Angle', 'Compl. Angle', 'Kalman Angle', 'Tilt Angle');
title('Angle');
grid on;

figure;
plot(Tempo, acel(:,1));
hold on;
plot(Tempo, acel(:,2), 'r');
hold on;
legend('X', 'Y');
title('Acel (mG)');
grid on;

gyro_steps = diff(angle_steps);
gyro_steps = horzcat(gyro_steps(1),gyro_steps);
figure;
plot(Tempo, gyro(:,3), 'k');
hold on;
plot(Tempo, gyro_steps, 'b');
hold on;
legend('Gyro IMU', 'Pantilt');
title('Gyro (�/s)');
grid on;