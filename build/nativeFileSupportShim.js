var backgroundMsgSupport = {
	launchFileSelect : function(index, callback){
		chrome.extension.sendRequest({key: 'launchFileSelect', index: index}, function(response){
			callback(response);
		});
	},
}
