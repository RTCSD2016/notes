% Plot belt axis data
% File contains: t x1set x1 x2set x2 v1 v2

load('working\gluedata.txt');
subplot(2,2,1);
plot(gluedata(:,1),gluedata(:,2),'k--',gluedata(:,1),gluedata(:,3),'k');
ylabel('Position, belt #1');
xlabel('Time');
subplot(2,2,2);
plot(gluedata(:,1),gluedata(:,4),'k--',gluedata(:,1),gluedata(:,5),'k');
ylabel('Position, belt #2');
xlabel('Time');
subplot(2,2,3);
plot(gluedata(:,1),gluedata(:,6),'k');
ylabel('Velocity, belt #1');
xlabel('Time');
subplot(2,2,4);
plot(gluedata(:,1),gluedata(:,7),'k');
ylabel('Velocity, belt #2');
xlabel('Time');
