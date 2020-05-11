img= imread('window.jpg');
img= rgb2gray(img);
imshow(mat2gray(img));
[M N] = size(img);
 
%dot=Get2DPoints('test.jpg', 4);       
dot=ginput(4);                                             % click four points, top left, top right, bottom left, bottom right
w=round(sqrt((dot(1,1)-dot(2,1))^2+(dot(1,2)-dot(2,2))^2));     %Obtain the new rectangle width from the original quadrilateral
h=round(sqrt((dot(1,1)-dot(3,1))^2+(dot(1,2)-dot(3,2))^2));     %Obtain the height of the new rectangle from the original quadrilateral
 
y=[dot(1,1) dot(2,1) dot(3,1) dot(4,1)];        %Four original vertices
x=[dot(1,2) dot(2,2) dot(3,2) dot(4,2)];
%This is the new vertex, here chose rectangle
Y=[dot(1,1) dot(1,1) dot(1,1)+h dot(1,1)+h];     
X=[dot(1,2) dot(1,2)+w dot(1,2) dot(1,2)+w];
 
B=[X(1) Y(1) X(2) Y(2) X(3) Y(3) X(4) Y(4)]';   %The four transformed vertices, the right hand side of the equation
%Simultaneous solution of equations, coefficients of equations
A=[x(1) y(1) 1 0 0 0 -X(1)*x(1) -X(1)*y(1);             
0 0 0 x(1) y(1) 1 -Y(1)*x(1) -Y(1)*y(1);
   x(2) y(2) 1 0 0 0 -X(2)*x(2) -X(2)*y(2);
0 0 0 x(2) y(2) 1 -Y(2)*x(2) -Y(2)*y(2);
   x(3) y(3) 1 0 0 0 -X(3)*x(3) -X(3)*y(3);
0 0 0 x(3) y(3) 1 -Y(3)*x(3) -Y(3)*y(3);
   x(4) y(4) 1 0 0 0 -X(4)*x(4) -X(4)*y(4);
0 0 0 x(4) y(4) 1 -Y(4)*x(4) -Y(4)*y(4)];
 
fa=inv(A)*B;        %The solution of the four points is also the global transformation coefficient
a=fa(1);b=fa(2);c=fa(3);
d=fa(4);e=fa(5);f=fa(6);
g=fa(7);h=fa(8);
 
rot=[d e f;
     a b c;
     g h 1];    
 %homo perspectives
 rot1=[1 0 2;
     0 -1 2;
     0 0 1];
  rot2=[-2 0 5;
     0 2 5;
     0 0 1];
 
pix1=rot*[1 1 1]'/(g*1+h*1+1); 
pix2=rot*[1 N 1]'/(g*1+h*N+1); 
pix3=rot*[M 1 1]'/(g*M+h*1+1);  
pix4=rot*[M N 1]'/(g*M+h*N+1);  
 
height=round(max([pix1(1) pix2(1) pix3(1) pix4(1)])-min([pix1(1) pix2(1) pix3(1) pix4(1)]));     %The height of the transformed image
width=round(max([pix1(2) pix2(2) pix3(2) pix4(2)])-min([pix1(2) pix2(2) pix3(2) pix4(2)]));      %The width of the transformed image
imgn=zeros(height,width);
 
delta_y=round(abs(min([pix1(1) pix2(1) pix3(1) pix4(1)])));            %Gets the offset from the negative axis in the y direction
delta_x=round(abs(min([pix1(2) pix2(2) pix3(2) pix4(2)])));            %Gets the offset from the negative axis in the x direction
inv_rot=inv(rot);
 
for i = 1-delta_y:height-delta_y                        %Reverse search the original image from the transformed image to avoid voids
    for j = 1-delta_x:width-delta_x
        pix=inv_rot*[i j 1]';       
        pix=inv([g*pix(1)-1 h*pix(1);g*pix(2) h*pix(2)-1])*[-pix(1) -pix(2)]'; 
        
        if pix(1)>=0.5 && pix(2)>=0.5 && pix(1)<=M && pix(2)<=N
            imgn(i+delta_y,j+delta_x)=img(round(pix(1)),round(pix(2)));     %×îÁÚ½ü²åÖµ
        end  
    end
end
 
figure,imshow(uint8(imgn));
title('projective');

