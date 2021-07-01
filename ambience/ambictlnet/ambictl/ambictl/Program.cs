using System;
using System.Collections.Generic;
using Ambience;
using Ambience.Groups;

var calculator_mod = new LidlModule("/home/tos/ambience/services/interfaces/calc.lidl", "calc_schema");
var logger_mod = new LidlModule("/home/tos/src/core/log.yaml", "log_schema");
var alarm_mod = new LidlModule("/home/tos/ambience/services/interfaces/alarm.lidl", "alarm_schema");
var fs_mod = new LidlModule("/home/tos/ambience/services/interfaces/file_system.lidl", "filesystem_schema");

var calc_if = calculator_mod.GetService("tos::ae::services::calculator");
var logger_if = logger_mod.GetService("tos::services::logger");
var alarm_if = alarm_mod.GetService("tos::ae::services::alarm");
var fs_if = fs_mod.GetService("tos::ae::services::filesystem");

var basic_calc = calc_if.Implement("basic_calc", cmakeTarget: "basic_calc", sync: false, external: false,
    dependencies: new Dictionary<string, ServiceInterface>
        {{"logger", logger_if}, {"alarm", alarm_if}, {"fs", fs_if}});

var logger_impl = logger_if.Implement("logger", sync: true, external: true);
var alarm_impl = alarm_if.Implement("alarm", sync: false, external: true);
var fs_impl = fs_if.Implement("fs", sync: true, external: true);

var logger = logger_impl.Instantiate("logger");
var alarm = alarm_impl.Instantiate("alarm");
var fs = fs_impl.Instantiate("fs");


var calc = basic_calc.Instantiate("calc",
    dependencies: new Dictionary<string, ServiceInstance>
        {{"logger", logger}, {"alarm", alarm}, {"fs", fs}});


var g1 = new UserGroup("sample_group3", new HashSet<ServiceInstance> {calc});

Console.WriteLine(g1.OrderedInterfaceDependencies());
Console.WriteLine(g1.GenerateExternalDepsSection());

foreach (var x in g1.GenerateInitSection())
{
    Console.WriteLine(x);
}

Console.WriteLine(await g1.GenerateBody());