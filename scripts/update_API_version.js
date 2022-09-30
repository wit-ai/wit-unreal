var fs = require('fs');
const KEY_STRING = 'FString ApiVersion{"';

function readWriteAsync() {
  fs.readFile('..\\Source\\Wit\\Public\\Wit\\Configuration\\WitAppConfiguration.h', 'utf-8', function(err, data){
    if (err) throw err;
    let index = data.indexOf(KEY_STRING);
    if(index === -1) {
      console.log('Cannot find API version, please check WitAppConfiguration.h');
      return;
    }
    index = index + KEY_STRING.length;
    const version = getLatestVersion()
    console.log(`Going to update version to ${version}`);

    data = data.substring(0, index) + version + data.substring(index + version.length);

    fs.writeFile('..\\Source\\Wit\\Public\\Wit\\Configuration\\WitAppConfiguration.h', data, 'utf-8', function (err) {
      if (err) throw err;
      console.log('filelistAsync complete');
    });
  });
}

function getLatestVersion() {
  let date_ob = new Date();
  let date = ("0" + date_ob.getDate()).slice(-2);
  let month = ("0" + (date_ob.getMonth() + 1)).slice(-2);
  let year = date_ob.getFullYear();
  return `${year}${month}${date}`
}

readWriteAsync()
