%Import and gray images
I1 = imread('wy.jpg');
I2 = imread('wjkt.jpg');
I1=rgb2gray(I1);
I2=rgb2gray(I2);
%I1=im2double(I1); 
%I2=im2double(I2); 

%i
f1=fftn(I1);
mag1=abs(f1);
phase1=angle(f1);
%image b
f2=fftn(I2);
mag2=abs(f2);
phase2=angle(f2);

r1=ifftshift(ifftn(mag1));
r2=ifftn(exp(1i*phase1));
r3=ifftshift(ifftn(mag2));
r4=ifftn(exp(1i*phase2));

%exchange
ab=mag1.*cos(phase2)+mag1.*sin(phase2).*i;
ba=mag2.*cos(phase1)+mag2.*sin(phase2).*i;

abr=abs(ifft2(ab));
bar=abs(ifft2(ba));
abr=uint8(abr);
bar=uint8(bar);

%i result
figure,subplot(2,3,1),imshow(I1);title('image A');
subplot(2,3,2),imshow(uint8(r1));title('A magnitude-only reconstruction');
subplot(2,3,3),imshow(r2,[]);title('A phase-only reconstruction');
subplot(2,3,4),imshow(I2);title('image B');
subplot(2,3,5),imshow(uint8(r3));title('B magnitude-only reconstruction');
subplot(2,3,6),imshow(r4,[]);title('B phase-only reconstruction');

%II
figure,subplot(2,2,1),imshow(I1);title('image A');
subplot(2,2,2),imshow(abr,[]);title('reconstructed image from (ii)(a)');
subplot(2,2,3),imshow(I2);title('image B');
subplot(2,2,4),imshow(bar,[]);title('reconstructed image from (ii)(b)');






