function [avg,minn,maxx,stdd,times,k] = anlz_trl(trail,task,from,to,nskip)
% Analyze a transition trail by finding specific patterns and computin
% time differences
% trail - the audit trail to be analyzed
% task, to , from - the pattern to identify in the audit trail (ignore item
%		if < 0)
% nskip - number of initial occurences to skip

k = 0;  % Count of matches
ksk = 0;

n = size(trail,1);
for i = 1:n
	if ( (trail(i,2) == task) | ( task < 0)) & ((trail(i,3) == from) | (from < 0)) ...
		 & ((trail(i,4) == to) | (to < 0))
		if ksk >= nskip	% Skip first 'nskip' of these
			% This is a match
			k = k + 1;
			times(k) = trail(i,1);	% Record time
		else
			ksk = ksk + 1;
		end
	end
end
tdif = abs(diff(times));	% Time differences (absolute values)
maxx = max(tdif);
minn = min(tdif);
avg = mean(tdif);
stdd = std(tdif);
