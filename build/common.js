var p = document.createElement("embed");
p.setAttribute("type", "application/x-fbod");
document.documentElement.appendChild(p);

/*var backgroundMsgSupport = {
	launchFileSelect : function(index, callback){
		chrome.extension.sendRequest({key: 'launchFileSelect', index: index}, function(response){
			callback(response);
		});
	},
}
function launchFileSelect() {
        return backgroundMsgSupport.launchFileSelect()
}
*/
