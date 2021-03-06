% Matlab script file to plot PWM output
% This file plots a single PWM signal
% File: pwm_plot.m
% Created 12/25/95, DM Auslander

load pwm_out.txt  % Read in the general output file
nv = length(pwm_out);
subplot(2,1,1);
plot(pwm_out(:,1),pwm_out(:,2),'k');
ylabel('Duty Cycle');
subplot(2,1,2);
plot(pwm_out(:,1),pwm_out(:,3),'k');
axis([0 pwm_out(nv,1)  0 1.5]);
xlabel('Time'); ylabel('PWM Out');
