function XY2D=Get2DPoints(ImageFileName, NumberOfPoints)
XY2D=[];
[Img, Col]=imread(ImageFileName, 'jpg'); %%% assuming the imagefile is a jpg file.
image(Img); drawnow; hold on;
for i=1:NumberOfPoints
[x, y]=ginput(1);
v=[x;y];
plot(x, y, 'r*');
XY2D=[XY2D v];
end
return;

