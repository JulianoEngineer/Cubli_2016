close all;
clear all;
delete(instrfindall)

%Set Sine Period
Tp =    1;
Ts =    0.02;
waveIteration = 5;
tilt_res = 4;
max_angle = 30;

[tilt_steps, angle_steps, Tempo, tilt_index] = SineTiltGenerate(max_angle,Ts,Tp,waveIteration,4);

tilt_send = tilt_steps(tilt_index);
angle_send = angle_steps(tilt_index);
Tempo_send = Tempo(tilt_index);

%Open Serial Port:
s = serial('COM5','BaudRate', 9600, 'DataBits', 8, 'StopBits', 1, 'Parity', 'none', 'Timeout', 3, 'Terminator', 'CR/LF');
fclose(s);
fopen(s);

%Send initial position:
disp('Set Initial Tilt Position');
str_tiltpos = sprintf('TP%.0f',tilt_steps(1));
fprintf(s,'%s\r',str_tiltpos);
out = fscanf(s);
pause(1);

disp('Sending Tilt Positions');

j = 1;
for i = 1:length(tilt_steps)
    
    str_tiltpos = sprintf('TP%.0f',tilt_steps(i));
    if Tempo_send(j) == Tempo(i)
        fprintf(s,'%s\r',str_tiltpos);
        fscanf(s);
        j = j + 1;
    end
    pause(Ts);
    
end

fprintf(s,'%s\r',str_tiltpos);
fscanf(s);

fclose(s);
delete(s);

% figure;
% plot(Tempo, tilt_steps, '-*');
% hold on;
% plot(Tempo_send, tilt_send, '-*r');
% grid on;
% title('Tilt Steps Positions');

figure;
plot(Tempo, angle_steps, '-*');
hold on;
plot(Tempo_send, angle_send, '-*r');
grid on;
title('Tilt Angle Positions');

gyro_steps = diff(angle_steps);
gyro_steps = horzcat(gyro_steps(1),gyro_steps);
figure;
plot(Tempo, gyro_steps, '-*k');
grid on;
title('Tilt Gyro');