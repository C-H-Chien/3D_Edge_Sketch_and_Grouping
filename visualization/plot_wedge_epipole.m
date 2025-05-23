clear;
close all;

% Validation View Indices
validation_view_indices = [49];

% Epipoles for Hypothesis 1
epipoles_hypo1 = [ 
    181.227 -2251.84
];

% Angle Ranges for Hypothesis 1
angle_ranges_hypo1 = [
    82.7941, 82.8215
];

epipoles_hypo2 = [
    7482.74, -6722.24
];

angle_ranges_hypo2= [
    134.226, 134.232
];

Dataset_Path = "/gpfs/data/bkimia/Datasets/ABC-NEF/";
Object_Name = "00000006/";
Image_Path = fullfile(Dataset_Path, Object_Name, "train_img/");

% Image dimensions
image_width = 800;
image_height = 800;


figure;
for i = 1:length(validation_view_indices)
    % subplot(2, ceil(length(validation_view_indices) / 2), i);
    image_file = fullfile(Image_Path, sprintf("%d_colors.png", validation_view_indices(i)));
    img = imread(image_file);
    imshow(img);
    hold on;

    hyp1_epipole = epipoles_hypo1(i, :);
    hyp1_angle_deg_1 = angle_ranges_hypo1(i, 1);
    hyp1_angle_deg_2 = angle_ranges_hypo1(i, 2);
    plotLineOnImage(hyp1_epipole, hyp1_angle_deg_1, image_width, 'r-');
    plotLineOnImage(hyp1_epipole, hyp1_angle_deg_2, image_width, 'r-');

    hyp2_epipole = epipoles_hypo2(i, :);
    hyp2_angle_deg_1 = angle_ranges_hypo2(i, 1);
    hyp2_angle_deg_2 = angle_ranges_hypo2(i, 2);
    plotLineOnImage(hyp2_epipole, hyp2_angle_deg_1, image_width, 'b-');
    plotLineOnImage(hyp2_epipole, hyp2_angle_deg_2, image_width, 'b-');

    % Add a label to indicate the Validation View Index
    text(20, 20, ['Validation View Index: ', num2str(validation_view_indices(i))], ...
        'FontSize', 8, 'Color', 'green', 'FontWeight', 'bold');
    plot(519.6032,  430.9860,'o', 'Color', 'c', 'MarkerSize', 7, 'LineWidth', 1);
end


function plotLineOnImage(epipole, angle_deg, image_width, type)

    x = linspace(0, image_width, 1000);
    angle_rad = deg2rad(angle_deg);
    slope = tan(angle_rad);
    x1 = epipole(1);
    y1 = epipole(2);

    y = y1 + slope * (x - x1);
    plot(x, y, type, 'LineWidth', 2); % 'r-' specifies a red solid line
    hold on;
end

