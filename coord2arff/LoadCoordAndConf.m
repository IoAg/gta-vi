% function LoadCoordAndConf:
% This function Loads data from the given input file and returns a data matrix. 
% 
% input:
%   inputFile           - file that holds data to  load
% output:
%   data     - data matrix holding timestamp and coordinates nx3 where confidence > 0.1
%   pixelX   - width in pixels
%   pixelY   - height in pixels
%   width    - width of monitor in meters
%   height   - height of monitor in meters
%   distance - distance from monitor in meters
%   conf     - confince of return data. In range [0,1]

function [data, pixelX, pixelY, width, height, distance, conf] = LoadCoordAndConf(inputfile)
    pixelX = -1;
    pixelY = -1;
    width = -1;
    height = -1;
    distance = -1;

    inputfileClean = strtrim(inputfile);
    if (exist(inputfileClean) == 0)
        error('File does not exist.');
    endif
    
    % check if input file has header
    fid = fopen(inputfileClean);
    numOfHeaderLines = 0;
    for i=1:20
        l = fgetl(fid);
        if (!ischar(l)) % detect end of file
            break;
        end
        pos = strfind(l, 'geometry');
        if (size(pos,1) > 0)
            numOfHeaderLines = i;
        end
    end
    fclose(fid);

    if (numOfHeaderLines > 0)
        data = importdata(inputfileClean, ' ', numOfHeaderLines);
        matrix = data.data;
        s = strsplit(data.textdata{numOfHeaderLines}, ' ');
        distance = str2num(s{1,3});
        width = str2num(s{1,5});
        height = str2num(s{1,7});
        s = strsplit(data.textdata{numOfHeaderLines-1}, ' ');
        pixelX = str2num(s{1,2});
        pixelY = str2num(s{1,3});
    end

    data = importdata(inputfileClean, ' ', numOfHeaderLines);
    [rows, columns] = size(data.data);
    if (columns ~= 4 & columns ~= 13)
        error("Wrong type of input file.");
    end

    conf = data.data(:,4);
    data = data.data(:,1:3);
end
