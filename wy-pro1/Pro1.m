I=imread('enheng.png');
figure,imshow(I);
title('srcImage');
I1=imrotate(I,30);                  %rotate 30��using imrotate
I2=imrotate(I,60);                  %rotate 60��using imrotate
I3=myimrotate(I,30);     %rotate 30�� using myimrotate() 
I4=myimrotate(I,60);     %rotate 30�� using myimrotate() 
figure,imshow(I1);
title('matlab I1');
figure,imshow(I2);
title('matlab I2');
figure,imshow(uint8(I3));
title('rotate by 30�㣺I3');
figure,imshow(uint8(I4));
title('rotate by 60�㣺I4');




