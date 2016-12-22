function [dc_avg,dc_min,dc_max,dc_std,dc,n] = msr_duty(data,nskip)
% Calculate the actual duty cycle attained
% This program is only intended for analysis of PWMs with duty cycles
% that are >1 and <0
% 'Data' contains [time valu] pairs for each change in PWM output

nd = length(data);
icycle = 0;		% Number of cycles counted
begin = 0;		% State indicator:
			% 0 - first entry, looking for cycle start
			% 1 - found cycle start
			% 2 - found beginning of OFF

% A cycle is defined as beginning when a '1' is encountered in 'data'
for  i = (nskip + 1) : nd
	if begin == 0
		% Look for a cycle start
		if data(i,2) == 1
			begin = 1;
			ibegin = i;
		end
	else 
		% Normal branch -- not the initial entry
		if begin == 1	% Looking for a 0 to begin OFF time
			if data(i,2) ~= 0
				'Value of 1 found unexpectedly'
				return;
			end
			% Found beginning of OFF time
			ioff = i;
			begin = 2;
		else
			% begin is 2 -- value should be 1
			if data(i,2) ~= 1
				'Value of 0 found unexpectedly'
				return;
			end
			icycle = icycle + 1;
			period = data(i,1) - data(ibegin,1);
			dutycycle = (data(ioff,1) - data(ibegin,1)) / period;
			dc(icycle, :) = [dutycycle period];
			begin = 1;		% Look for next cycle
			ibegin = i;
		end
	end
end

dc_avg = mean(dc);
dc_min = min(dc);
dc_max = max(dc);
dc_std = std(dc);
n = icycle;
