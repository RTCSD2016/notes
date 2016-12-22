% Plot the sequence of states from a transition audit file

load('working\trl_num.txt');

% Get scales
minval = min(trl_num);
maxval = max(trl_num);

axis([0 maxval(1)+2 0 maxval(2)+3]);
n = length(trl_num);

for i = 1:n
	text(trl_num(i,1),trl_num(i,2),num2str(trl_num(i,4)));
end
xlabel('Time');
ylabel('Task #');
title('State Sequences');
