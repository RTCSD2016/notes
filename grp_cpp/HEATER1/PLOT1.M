subplot(2,1,1);
plot(output(:,1),output(:,2),'k--',output(:,1),output(:,4),'k');
xlabel('Time, sec');
ylabel('Temp(solid), Setpt(dash)');
subplot(2,1,2);
plot(output(:,1),output(:,3),'k');
xlabel('Time, sec');
ylabel('Duty Cycle');

