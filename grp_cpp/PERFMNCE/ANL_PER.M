% Analyze PWM period information
% Script file: anl_per.m

%load trl_num.txt
%[avg,minn,maxx,stdd,times,k] = anlz_trl(trl_num,0,3,1,1);
%tdif=diff(times);
%plot(tdif-0.1,'k')
%xlabel('Sample #');
%ylabel('Error, sec');

load pwm_rec.txt
[dc_avg,dc_min,dc_max,dc_std,dc,n] = msr_duty(pwm_rec,2);
subplot(2,1,1);
plot(dc(:,1) - 0.45,'k');
ylabel('Error');
title('   Duty Cycle Error (desired value=0.45)');
subplot(2,1,2);
plot(dc(:,2) - 0.1,'k');
ylabel('Error, sec');
xlabel('Sample #');
title('   Period Error (desired value=0.1 sec)');

% Print statistics
fprintf(1,'      Duty Cycle    Period\n');
fprintf(1,'Avg  %10.5f %10.5f\n',dc_avg);
fprintf(1,'Std  %10.5f %10.5f\n',dc_std);
fprintf(1,'Min  %10.5f %10.5f\n',dc_min);
fprintf(1,'Max  %10.5f %10.5f\n',dc_max);
