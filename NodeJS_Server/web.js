const express = require('express');
const app = express();
const url = require('url');
const querystring = require('querystring');
const mysql = require('mysql');
const request = require('request')
const fs = require('fs');

let connection = mysql.createConnection({
    host: '149.28.29.84',
    user : 'maker',
    password : 'maker#$lora',
    database: 'Maker_LoRa'
})

//connection.connect();
app.use(express.static('public'));

app.get('/get/', (req, res) => {
  var fullUrl = req.protocol + '://' + req.get('host') + req.originalUrl;
  parsedobj = url.parse(fullUrl);
  parsedobj = querystring.parse(parsedobj.query);

  if(parsedobj.status=='set'){

    sqlquery = 'INSERT INTO `data` (`debug`, `data_num`, `data_temp`,`data_humid` ,`data_dust`, `data_co2`, `data_pres`,`data_batt`,`timeforme`) VALUES (?,?,?,?,?,?,?,?,?)';
    // ?temp=<TEMP>&dust=<DUST>&co2=<CO2>&pres=<PRESSURE>&batt=<BATTERY>&num=<NUM>
    //http://lora.cafe24app.com/get/?temp=20&dust=100&co2=0&pres=100&batt=33&num=3&status=set
    if(Number(parsedobj.num)==NaN||parseFloat(parsedobj.temp)==NaN||parseFloat(parsedobj.humid)==NaN||parseFloat(parsedobj.dust)==NaN||parseFloat(parsedobj.pres)==NaN){
        res.end("err")
    }
    else{
        let params = [500,Number(parsedobj.num),parseFloat(parsedobj.temp),parseFloat(parsedobj.humid),parseFloat(parsedobj.dust),0.0,parseFloat(parsedobj.pres),100,(new Date().yyyymmdd())+(time_format(new Date()))];
        connection.query(sqlquery,params, function (error, results, fields) {
            if (error) {
                console.log(error);
            }
            else{
                console.log('Success!');
            }
    });
    res.end('ACK');

    }
    
  }

  else{
      res.writeHead(200,{'Content-Type':'text/html'});
      res.end('Handle'); 
    }   
});

app.get('/',(req,res)=>{

    let sqlquery = 'SELECT * FROM data';
    fs.readFile("index.html", function (error, pgResp) {
        if (error) {
            console.log(error)
            res.writeHead(404);
            res.write('Contents you are looking are Not Found');
        } else {
            res.writeHead(200, { 'Content-Type': 'text/html' });
            res.write(pgResp);
        }
        res.end();
    });
})

app.get('/bob/',(req,res)=>{
    var date = new Date();
    
    request.get('https://api.dimigo.in/dimibobes/'+date.yyyymmdd(),{},(errror,response,body)=>{
        res.send(body)
        res.end();
    })

})

app.get('/graph/',(req,res)=>{
    let fullUrl = req.protocol + '://' + req.get('host') + req.originalUrl;
    parsedobj = url.parse(fullUrl);
    parsedobj = querystring.parse(parsedobj.query);
    let cless = parsedobj.class[0]
    date = new Date().yyyymmdd();
    let sqlquery = "SELECT * FROM `data` WHERE `data_num`="+cless+" AND `timeforme` LIKE ?";
    
    connection.query(sqlquery,String(date) + '%', function (error, results, fields) {
        if (error) {
            console.log(error);
        }
        else{
            res.send(results)            
            res.end('');
        }
    });
})



app.listen(8002, () => {
  console.log('Application On!');
});



Date.prototype.yyyymmdd = function() {
    var mm = this.getMonth() + 1; // getMonth() is zero-based
    var dd = this.getDate();
    
    return [this.getFullYear(),
            (mm>9 ? '' : '0') + mm,
            (dd>9 ? '' : '0') + dd
            ].join('');
};


function time_format(d) {
    let hours = format_two_digits(d.getHours());
    let minutes = format_two_digits(d.getMinutes());
    let seconds = format_two_digits(d.getSeconds());
    return hours + minutes + seconds;
}

function format_two_digits(n) {
    return n < 10 ? '0' + n : n;
}