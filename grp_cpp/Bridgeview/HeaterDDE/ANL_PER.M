% Analyze PWM period information
% Script file: anl_per.m

load trl_num.txt
[avg,minn,maxx,stdd,times,k] = anlz_trl(trl_num,3,1,1,1);
tdif=diff(times);
hist(tdif);
xlabel('Time Between Transitions, sec');
ylabel('Number of Occurrences');
title('Operator Interface Task, Char-OpInt');
