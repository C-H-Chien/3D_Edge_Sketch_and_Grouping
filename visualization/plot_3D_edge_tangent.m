clear;
clc;
close all;

%> Define the folder path containing the 3D edge and tangent output files
data_folder_name = 'outputs';
data_folder_path = fullfile(fileparts(mfilename('fullpath')), '..', data_folder_name);

%> Specify the common patterns in the file names
edge_file_pattern = "3D_edges*.txt";
tangent_file_pattern = "3D_tangents*.txt";

%> Get all files matching the patterns
edge_files = dir(fullfile(data_folder_path, edge_file_pattern));
tangent_files = dir(fullfile(data_folder_path, tangent_file_pattern));

%> Define a set of colors to be used for different files through 'lines' colormap
colors = lines(length(edge_files)); 

%> Scalar for tangent vectors
mag = 0.0025; 

target_edge_1 = [0.460003, 0.0259107, 0.352056];
target_edge_2 = [0.456393, 0.0307521, 0.346758];


%> Create a figure for plotting
figure;
hold on;

%> Loop through each file and plot its edges and tangents in 3D
for i = 1:length(edge_files)
 %for i = 4:4
    %> Read the current edge file
    current_edge_file_path = fullfile(data_folder_path, edge_files(i).name);
    edge_file_read = fopen(current_edge_file_path, 'r');
    disp(['Reading edges: ', current_edge_file_path]);

    %> Parse 3D edge data
    edge_data = textscan(edge_file_read, '%f\t%f\t%f', 'CollectOutput', true);
    edges_3d = double(edge_data{1, 1});
    fclose(edge_file_read);

    %> Find the corresponding tangent file
    current_tangent_file_path = fullfile(data_folder_path, tangent_file_pattern);
    if ~exist(current_tangent_file_path, 'file')
        disp(['Tangent file not found for: ', current_edge_file_path]);
        continue;
    end

    %> Read the corresponding tangent file
    tangent_file_read = fopen(current_tangent_file_path, 'r');
    disp(['Reading tangents: ', current_tangent_file_path]);
    tangent_data = textscan(tangent_file_read, '%f\t%f\t%f', 'CollectOutput', true);
    tangents_3d = double(tangent_data{1, 1});
    fclose(tangent_file_read);

    %> Get the legend
    hypothesis_view1_index = extractBetween(edge_files(i).name, 'hypo1_', '_hypo2');
    hypothesis_view2_index = extractBetween(edge_files(i).name, 'hypo2_', '_t');
    show_legend = strcat("3D edges from hypothesis views ", hypothesis_view1_index, " and ", hypothesis_view2_index);

    %> Plot the edges using a different color for each file
    plot3(edges_3d(:,1), edges_3d(:,2), edges_3d(:,3), ...
          'Color', colors(1, :), 'Marker', '.', 'LineStyle', 'none', ...
          'LineWidth', 0.1, 'MarkerSize', 3, 'DisplayName', show_legend);


    %> Check and highlight specific points
    for j = 1:size(edges_3d, 1)
    % 
        P = edges_3d(j, :)'; % Current edge point
        T_v = tangents_3d(j, :)'; % Corresponding tangent vector

        % Plot tangent vector as a line originating from the point, no legend
        line([P(1) + mag*T_v(1), P(1) - mag*T_v(1)], ...
             [P(2) + mag*T_v(2), P(2) - mag*T_v(2)], ...
             [P(3) + mag*T_v(3), P(3) - mag*T_v(3)], ...
             'Color', 'k', 'LineWidth', 0.5, 'HandleVisibility', 'off'); % Exclude from legend
        % point = edges_3d(j, :);
        % if (abs(point(1) - target_edge_1(1)) < 0.000001 && abs(point(2) - target_edge_1(2)) < 0.000001 && abs(point(3) - target_edge_1(3)) < 0.000001)
        %     disp('found 1');
        %     plot3(point(1), point(2), point(3), 'Color', 'r','Marker','o','MarkerSize', 5, 'LineWidth', 0.1, 'MarkerFaceColor', 'r');
        % end
        % if(abs(point(1) - target_edge_2(1)) < 0.000001 && abs(point(2) - target_edge_2(2)) < 0.000001 && abs(point(3) - target_edge_2(3)) < 0.000001)
        %     disp('found 2');
        %     plot3(point(1), point(2), point(3), 'Color', 'r','Marker','o','MarkerSize', 5, 'LineWidth', 0.1, 'MarkerFaceColor', 'r');
        % end
    end
    view(3);
end

%> Set the plot settings
axis equal;
axis off;
set(gcf, 'color', 'w');
ax = gca;
ax.Clipping = "off";

%> Add a legend for each file
legend;

hold off;
