% Plot oven data
% File format is t th1set th1 m1 th2set th2 m2

load('working\ovendata.txt');
subplot(2,2,1);
plot(ovendata(:,1),ovendata(:,4),'k');
ylabel('Heater#1 current');
subplot(2,2,3);
plot(ovendata(:,1),ovendata(:,2),'k--',ovendata(:,1),ovendata(:,3),'k');
ylabel('Heater#1 temperature');
subplot(2,2,2);
plot(ovendata(:,1),ovendata(:,7),'k');
ylabel('Heater#2 current');
subplot(2,2,4);
plot(ovendata(:,1),ovendata(:,5),'k--',ovendata(:,1),ovendata(:,6),'k');
ylabel('Heater#2 temperature');
