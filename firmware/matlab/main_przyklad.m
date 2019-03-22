close all;
clear; clc;
%%
% Sync_Send('init');
% Sync_Send('done');
% Sync_Send('init');
% Sync_Send('init');
% Sync_Send('done');

while(1)
    
Sync_Send('init');
Sync_Send(1, 'on');
Sync_Send(2, 'on');
Sync_Send(3, 'on');
Sync_Send(4, 'on');
Sync_Send(5, 'on');
Sync_Send(6, 'on');


Sync_Send('done');

end
