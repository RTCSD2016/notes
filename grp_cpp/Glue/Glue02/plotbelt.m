% Plot belt axis data
% File contains: t x1set x1 x2set x2

system_dependent(14,'on');  % For better consistency working with
  % word processors
load('working\gluedata.txt');
subplot(2,1,1);
plot(gluedata(:,1),gluedata(:,2),'k--',gluedata(:,1),gluedata(:,3),'k');
ylabel('Position, belt #1');
xlabel('Time');
subplot(2,1,2);
plot(gluedata(:,1),gluedata(:,4),'k--',gluedata(:,1),gluedata(:,5),'k');
ylabel('Position, belt #2');
xlabel('Time');
