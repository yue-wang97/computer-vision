function main
clear all;
close all;

black = imread('myhand.jpg');
white = imread('myface.jpg');
mask = imread('mymask.png');

% Convert mask to alpha mask
maskr = double(mask(:,:,1)) / 255;

% First image - Mask pixels that belong to label 0
black_im = uint8(zeros(size(black)));
for i = 1 : 3
    b = black(:,:,i);
    out = zeros([size(black,1) size(black,2)]);
    out(maskr == 0) = b(maskr == 0);
    black_im(:,:,i) = out;
end

black_im = uint8(black_im);

%%%% Now do Laplacian blending
outStitch2 = LaplacianBlend(black, white, mask);

% Show all the images individually
figure;
imshow(black);
title('First Image (myhand)');
figure;
imshow(white);
title('Second Image (myface)');
figure;
imshow(mask);
title('Mask used for blending');
figure;
imshow(outStitch2);
title('Output after Laplacian Blending');
end
%%
% Function that takes an r x c image, and creates an expanded version
% of size 2r x 2c. 
function [expout] = expand(im)
    % Allocate memory for expansion
    rows = size(im, 1);
    cols = size(im, 2);
    expout = zeros([2*rows 2*cols 3]);
    
    expout(1:2:2*rows,1:2:2*cols,:) = im;

    % Smooth the image
    ker = generating_kernel(5,1.5);
    expout = imfilter(expout, ker, 'conv');
    
    % Must scale by 4 to ensure that net weight contributing
    % to each output pixel is 1.
    expout = 4*expout;
end
%%
% Function that blends two images together using Laplacian Pyramids
function [out] = LaplacianBlend(black, white, mask)
    % Cast each of the images to double
    % matting
    black_img = double(black);
    white_img = double(white);
    mask_img = double(mask) / 255.0;
    
    % set pyramid_level=3  
    depth = 3;
    
    % Create Gaussian pyramids for the two images and the mask
    gauss_pyr_mask = gauss_pyramid(mask_img, depth);
    gauss_pyr_black = gauss_pyramid(black_img, depth);
    gauss_pyr_white = gauss_pyramid(white_img, depth);
    %--
    gauss_pyr_black0 = gauss_pyramid(black, depth);
    gauss_pyr_white0 = gauss_pyramid(white, depth);

    
    % Create the Laplacian pyramids for the two images
    lapl_pyr_black = lapl_pyramid(gauss_pyr_black);
    lapl_pyr_white = lapl_pyramid(gauss_pyr_white);
    lapl_pyr_mask = lapl_pyramid(gauss_pyr_mask);
    
    % Blend the two Laplacian pyramids together
    outpyr = blend(lapl_pyr_white, lapl_pyr_black, gauss_pyr_mask);
    
    % Create final blended image
    out = collapse(outpyr);
    
    % Cap and cast
    out(out < 0) = 0;
    out(out > 255) = 255;
    out = uint8(out);
end
%%
% Create a Gaussian pyramid
% Essentially this uses reduce for a total of levels times.
% Each image in one scale is half the size of the previous scale
% First scale is simply the original image
function [gaussout] = gauss_pyramid(im, levels)
    gaussout = cell(1,levels+1);
    gaussout{1} = im;
    figure,imshow(gaussout{1});title('gaussian_pyramid0'); 
    subsample = im;
    for i = 2 : levels+1
        subsample = reduce(subsample);
        gaussout{i} = subsample;
        figure,imshow(subsample);title('gaussian_pyramid'); 
    end 
end
%%
% Given a (blended) Laplacian pyramid, starting from the smallest scale,
% expand it, then add this previous scale.  Repeat this until we get to the
% original scale and this is the output
function [col_output] = collapse(lapl_pyr)
    lapl_pyr_copy = lapl_pyr;
    for i = length(lapl_pyr) : -1 : 2
        
        % Same Caveat for expanding is here
        tmp = expand(lapl_pyr_copy{i});
        rows = size(lapl_pyr_copy{i-1},1);
        cols = size(lapl_pyr_copy{i-1},2);
        tmp = tmp(1:rows, 1:cols, :);
        lapl_pyr_copy{i-1} = lapl_pyr_copy{i-1} + tmp;
    end

    col_output = lapl_pyr_copy{1};
end
%%
% Create a pyramid of blending two images together
% Given the Laplacian pyramids of two images as well as the mask
% image, for each scale apply an alpha blending equation

function [blended_pyr] = blend(pyr_white, pyr_black, pyr_mask)
    blended_pyr = cell(1, length(pyr_white));
    for i = 1 : length(pyr_white)
        whiteImg = pyr_white{i};
        blackImg = pyr_black{i};
        maskPyr = pyr_mask{i};

        blended_pyr{i} = maskPyr.*whiteImg + (1-maskPyr).*blackImg;
    end
end
%%
% Function that calculates a Laplacian pyramid
% Take an image in a Gaussian pyramid at a smaller scale (i+1) 
% and expand it
% Then subtract it with the i'th scale
% Image at the end of the pyramid is simply the last image in the
% Gaussian pyramid
function [pyramid] = lapl_pyramid(gauss_pyr)
    pyramid = cell(1,length(gauss_pyr));
    
    for i = 1 : length(gauss_pyr)-1
        % Expand i+1'th scale
        tmp = expand(gauss_pyr{i+1});
        
        % When expanding, we may get image dimensions that 
        % aren't quite the same when targetting a particular scale
        % As such, we need to make sure we are consistent
        rows = size(gauss_pyr{i},1);
        cols = size(gauss_pyr{i},2);
        tmp = tmp(1:rows,1:cols,:);
        
        % Take i'th scale and subtract with expanded i+1'th scale
        pyramid{i} = gauss_pyr{i} - tmp;
        figure,imshow(pyramid{i});title('laplacian_pyramid'); 
    end
    
    % Attach smallest scale from Gaussian pyramid in the end
    pyramid{end} = gauss_pyr{end};
end
%%
% Method that subsamples an image
% Given an r x c image, we blur the image first
% using the filter kernel produced by generating_kernel,
% the subsample the rows and columns by a factor of 2
function [redout] = reduce(im)
    % Create kernel
    ker = generating_kernel(5,1.5);

    % Blur the image
    img_blur = imfilter(im, ker, 'conv');

    % Subsample
    rows = size(im,1);
    cols = size(im,2);

    % Get every other row
    img_blur_rows = img_blur(1:2:rows,:,:);

    % Now get every other column
    redout = img_blur_rows(:,1:2:cols,:);
end
%%
% Function that generates a 5 x 5 filtering kernel
% used for Laplacian Pyramidal blending
% a is usually 0.4
function [h] = generating_kernel(p2, p3)
     siz   = (p2-1)/2;
     std   = p3;
     
     [x,y] = meshgrid(-siz:siz,-siz:siz);
     arg   = -(x.*x + y.*y)/(2*std*std);

     h     = exp(arg);
     h(h<eps*max(h(:))) = 0;

     sumh = sum(h(:));
     if sumh ~= 0,
       h  = h/sumh;
     end;
end

