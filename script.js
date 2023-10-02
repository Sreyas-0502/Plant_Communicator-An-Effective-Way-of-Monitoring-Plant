function getData()
{
    let request=new XMLHttpRequest();
    request.open("get","https://api.thingspeak.com/channels/2100510/feeds.json?results=1");
    request.send();
    request.onreadystatechange=function(){
        if (request.readyState == 4 && request.status == 200){
            let data=JSON.parse(request.responseText);
            console.log(data);
            document.getElementById("press").innerText=data.feeds[0].field1;
            document.getElementById("temp").innerText=data.feeds[0].field3;
            document.getElementById("humid").innerText=data.feeds[0].field2;
            document.getElementById("moisture").innerText=data.feeds[0].field4;
       }
    }
}

window.onload=getData;
window.setInterval(getData, 2000);