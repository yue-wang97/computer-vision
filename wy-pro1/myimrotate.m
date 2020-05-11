function [ A ] = myimrotate(I,degree)                                
[r,c,d]=size(I);                                                      %Gets row r, column c, and channel number d of input image B
H=round(r*abs(cosd(degree))+c*abs(sind(degree)));                    %The new height obtained by rotating the image
W=round(c*abs(cosd(degree))+r*abs(sind(degree)));                    %The new width obtained by rotating the image
A=zeros(H,W,d);                                                     %Initialize the output image
M1=[1 0 0;
    0 1 0;
    0.5*W -0.5*H 1 ];                                               %Coordinate transformation matrix M1
M2=[cosd(degree) -sind(degree) 0;
    sind(degree) cosd(degree) 0;
    0 0 1];                                                           %Rotation matrix M2
M3=[1 0 0;
    0 1 0;
    0.5*c 0.5*r 1];                                                   %Coordinate transformation matrix M3
    for i=1:W
        for j=1:H
            H1=[i j 1]*M1*M2*M3;                                    %get the transformed matrix H1
            y=H1(1,2);                                             
            x=H1(1,1);                                             
            y=round(y);                                              
            x=round(x);                                              
           if(x>=1&&x<=c)&&(y>=1&&y<=r)                             
               A(j,i,:)=I(y,x,:);                                    %Assign a value to the point on the rotated image 
           end                                                    
        end
    end
end


