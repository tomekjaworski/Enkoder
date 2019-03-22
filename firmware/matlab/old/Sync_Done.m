function  Sync_Done()

    global sync_obj;
    if (isempty(sync_obj))
        return;
    end;
    
    fclose(sync_obj.conn);
    sync_obj = [];

end

