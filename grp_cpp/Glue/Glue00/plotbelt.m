% Script file to plot data from the belt axis simulation

load('working\gluedata.txt');
system_dependent(14,'on');  % Try to avoid conflicts with word processors
subplot(2,2,1);
plot(gluedata(:,1),gluedata(:,2),'k');
ylabel('Position #1');

subplot(2,2,2);
plot(gluedata(:,1),gluedata(:,4),'k');
ylabel('Position #2');

subplot(2,2,3);
plot(gluedata(:,1),gluedata(:,3),'k');
ylabel('Velocity #1');
xlabel ('Time, sec');

subplot(2,2,4);
plot(gluedata(:,1),gluedata(:,5),'k');
ylabel('Velocity #2');
xlabel ('Time, sec');
