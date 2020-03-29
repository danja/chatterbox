
space = zeros(100,1000);
x = [1:1:1000];
y = [1:1:100];
# x = linspace (1, 1000, 1);
# y = linspace (1, 100, 1);

for i = 1:100
	k = 2 + (i-1) * 3 / 100;
	y(i) = k;
	space(i, 1) = 0.5;
	for j = 2:1000
		z = space(i, j-1);
		space(i, j) = k * z * (1-z);
	end
end

plot3(space(:,:))
plotmatrix (space, "g+");
xlabel ("r.*sin (t)");
ylabel ("r.*cos (t)");
zlabel ("z");
title ("Logistic Map");
