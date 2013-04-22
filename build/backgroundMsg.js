var backgroundMsgSupport = {
        launchFileSelect : function(index, url,  callback){
                chrome.extension.sendRequest({key: 'launchFileSelect', index: index, url: url}, function(response){callback(response)});
        }
};

