%> Automatically plot every 3D edge file in the folder with a different color
%
%> (c) LEMS, Brown University
%> chiang-heng chien
clear;
clc;
close all;

%> Define the folder path containing the 3D edge output files
data_folder_name = 'outputs';
data_folder_path = fullfile(fileparts(mfilename('fullpath')), '..', data_folder_name);

%> Specify the common pattern in the file names
file_pattern = "triangulated_3D_edges*hypo1_*.txt";

%> Get all files matching the pattern
edge_files = dir(fullfile(data_folder_path, file_pattern));

%> Define a set of colors to be used for different files through 'lines' colormap
colors = lines(length(edge_files)); 

% Create a figure for plotting
figure;
hold on;

%> Loop through each file and plot its edges in 3D
for i = 1:length(edge_files)
    %> Read the current file
    current_file_path = fullfile(data_folder_path, edge_files(i).name);
    edges_file_read = fopen(current_file_path, 'r');
    disp(current_file_path);

    %> Parse 3D edge data
    ldata = textscan(edges_file_read, '%f\t%f\t%f', 'CollectOutput', true);
    edges_3d = double(ldata{1,1});
    fclose(edges_file_read);

    %> Get the legend
    hypothesis_view1_index = extractBetween(edge_files(i).name, 'hypo1_', '_hypo2');
    hypothesis_view2_index = extractBetween(edge_files(i).name, 'hypo2_', '_t');

    if contains(edge_files(i).name, "tri")
        show_legend = strcat("Triangulated 3D edges from hypothesis views ", hypothesis_view1_index, " and ", hypothesis_view2_index);
    else
        show_legend = strcat("3D edges from hypothesis views ", hypothesis_view1_index, " and ", hypothesis_view2_index);
    end

    %> Plot the edges using a different color for each file
    plot3(edges_3d(:,1), edges_3d(:,2), edges_3d(:,3), ...
          'Color', colors(i, :), 'Marker', '.', 'MarkerSize', 3, 'LineStyle', 'none', ...
          'DisplayName', show_legend);
end

%> Set the plot settings
axis equal;
axis off;
set(gcf, 'color', 'w');

%> Display the legend
legend;
