var ststmr = null;

$(document).ready(function(){
	updatefw();
	updateinfo();
	
	$("#form_pid").on("submit", function(e) {
		e.preventDefault();
		setpid(form2json($(e.target)));
		return true;
	});
});

function gettemp() {
	$.getJSON("/get_temp.json", function(data) {
		$("#temp").val(data["temp"]);
		$("#output").val(data["output"]);
	});
}

function updatefw() {
	$.getJSON("/get_fw.json", function(data) {
		$("#lblsetpoint").text("Setpoint �C (" + data["temp_min"] + "~" + data["temp_max"] + ")");
		$("#setpoint").attr({
		   "min" : data["temp_min"],
		   "max" : data["temp_max"]
		});
		$("#setpoint").val(data["setpoint"]);
		$("#kp").val(data["kp"]);
		$("#ki").val(data["ki"]);
		$("#kd").val(data["kd"]);
	});
}

function updateinfo() {
	gettemp();
	if (ststmr == null)
		ststmr = setInterval(updateinfo, 5000);
}

function setpid(jsonval) {
	if (confirm("Enviar nova configura��o PID?") == true) {
		$.ajax({
			url: "/set_pid.json",
			dataType: "json",
			method: "POST",
			cache: false,
			data: JSON.stringify(jsonval),
			success: function(result) {
				var response = JSON.parse(JSON.stringify(result));
				if (response.status == "ok") {
					alert("Ok");
				}
				else {
					alert("Erro nos dados");
				}
			},
			error: function() {
				alert("N�o foi poss�vel enviar a solicita��o");
			}
		});
	}
}

function form2json(form) {
	return $(form)
		.serializeArray()
		.reduce(function (json, { name, value }) {
			json[name] = value;
			return json;
		}, {});
}