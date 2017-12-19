// orazio JS simple javascript thing
var ws = new WebSocket('ws://'+self.location.host+'/','orazio-robot-protocol');
refresh_toggled=true;

ws.onmessage = function(event) {
    if (refresh_toggled==true){
        document.getElementById('msgBox').innerHTML = event.data;
	document.getElementById('outMsg').value='';
    }
}

function setVariable(variable)
{
    var id_name=variable.replace(/\./g,'_');
    id_name=id_name.replace(/\]/g,'_');
    id_name=id_name.replace(/\[/g,'_');
    
    console.log("setvariable " + id_name );
    ws.send("set " + variable + " "+ document.getElementById(id_name).value);
    refresh_toggled=true;
}

function sendPacket(packet)
{
    ws.send('send ' + packet);
    refresh_toggled=true;
}

function stopRefresh(packet){
    refresh_toggled=false;
}

function storeParams(packet){
    ws.send("store " + packet);
    refresh_toggled=true;  
}

function fetchParams(packet){
    ws.send("fetch " + packet);
    refresh_toggled=true;  
}

function requestParams(packet){
    ws.send("request " + packet);
    refresh_toggled=true;  
}
