% Simulation of timing errors for 
% interrupt dispatched tasks
% 10/27/95, D. M. Auslander

clock_freq = 1193180;	% Hz
int_desired = 0.00097;	% interrupt period, sec
ticks_per_int = round(clock_freq * int_desired);
int_actual = ticks_per_int / clock_freq;
task_period = 0.1;	% sec

n = 20000;
t = 0.0; next = 0; it = 0;
tvals=[]; tdes = [];

for i = 1:n
	t = t + int_actual;
	%if t >= next
	if t > (next - int_actual / 2)
		it = it + 1;
		tvals(it) = t;
		tdes(it) = next;
		%next = next + task_period;  % Absolute timing
		next = t + task_period;  % Relative timing
	end
end

df = diff(tvals);
dt = tdes - tvals;
%plot(df,'k');xlabel('Sample #');ylabel('Task Period');
disp('mean(intv)    min(intv)    max(intv)');
disp([mean(df) min(df) max(df)]);
