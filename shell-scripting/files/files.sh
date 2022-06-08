function move_file {
    local FROM=$1
    local TO=$2
   
    if [ -f $TO/$(basename $FROM) ]; then 
	    return 2
    elif [ ! -d $TO ]; then
        return 1
    else
        mv $FROM $TO
        return 0
    fi
}

function create_file {
    local FILE_NAME=$1
    if [ -f $FILE_NAME ]; then
        return 1
    else
        touch $FILE_NAME
        return 0
    fi
}

function list_files {
    ls
}

select action in 'List Files' 'Create files' 'Move files' 'Exit'
do
    case $action in
    'List Files')
        list_files
    ;;
    'Create files')
        read -p 'File name> ' FILE_NAME
        if [ ! -z $FILE_NAME ]; then
            create_file $FILE_NAME
            case $? in
            0) echo 'File created successfully';;
            1) echo 'A file already exists at the provided location';;
            esac
        fi
    ;;
    'Move files')
        read -p 'Move file> ' FILE_NAME
        if [ ! -z $FILE_NAME ]; then
            read -p 'To> ' DESTINATION
            if [ ! -z $DESTINATION ]; then
                move_file $FILE_NAME $DESTINATION
                case $? in
                0) echo 'File moved successfully';;
                1) echo 'The provided file is not a directory or does not exist';;
            	2) echo "A file named $(basename $FILE_NAME) already exists at the provided location";;
                esac
            fi
        fi
    ;;
    'Exit')
        echo 'Exiting...'
        exit 0
    ;;
    *)
        echo 'Unknown action'
    ;;
    esac
done

