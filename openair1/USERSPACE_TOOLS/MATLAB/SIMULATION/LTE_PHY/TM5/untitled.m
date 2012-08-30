t = 0:pi/20:2*pi;
y = exp(sin(t));
h = plot(t,y,'YDataSource','y');
for k = 1:.1:10
	y = exp(sin(t.*k));
	refreshdata(h,'caller') % Evaluate y in the function workspace
	drawnow; pause(.1)
end