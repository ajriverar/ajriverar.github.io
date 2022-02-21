const express = require('express');
const uuid = require('uuid');
const fs = require('fs')
const db = require('./db.json');

const app = express();

app.use(express.json());
app.use(express.urlencoded({extended: true}));
app.use(express.static('public'));

app.get('/tickets', (_, res) => {
  res.send(db);
});

app.get('/tickets/pendientes', (req, res) => {
  const filtradas = db.tickets.filter((el) => el.completada === false);
  res.send({
    tickets: filtradas,
  });
});

app.get('/ticket/:id', (req, res) => {
  const { id } = req.params;
  const ticket = db.tickets.find((el) => el.id === id);
  res.send({
    ticket,
  });
});

app.get('/borraticket/:id', (req, res) => {
  const { id } = req.params;
  const idx = db.tickets.findIndex((el) => el.id === id);
  db.tickets.splice(idx, 1);

  fs.writeFile('./db.json', JSON.stringify(db, null, 2), () => {} );
  res.send(db);
});

app.post('/nueva', (req, res) => {
  const { cliente, solicitud } = req.body;
  let f = new Date();
  const fechastr = f.getFullYear() + "-" + (f.getMonth()+1) + "-" + f.getDate();

  if (cliente && solicitud) {
    const nuevoTicket = {
      id: uuid.v4(),
      cliente,
      solicitud,
      fecha: fechastr,
      completada: false,
    };

    db.tickets.push(nuevoTicket);
    fs.writeFile('./db.json', JSON.stringify(db, null, 2), () => {} );
    res.redirect("/");
  } else {
    res.send({
      error: 'No se ha podido crear el ticket :(',
    });
  }
});

app.get('/actualizaticket/:id/:completada', (req, res) => {
  const {
    id,
    completada,
  } = req.params;

  const ticketIdx = db.tickets.findIndex((el) => el.id === id);

  db.tickets[ticketIdx].completada = completada === 'true';

  fs.writeFile('./db.json', JSON.stringify(db, null, 2), () => {} );
  res.send({});
});

app.post('/editar', (req, res) => {
  const {
    cliente,
    fecha,
    solicitud,
    completada,
    id,
  } = req.body;

  const ticketIdx = db.tickets.findIndex((el) => el.id === id);

  db.tickets[ticketIdx].cliente = cliente;
  db.tickets[ticketIdx].fecha = fecha;
  db.tickets[ticketIdx].solicitud = solicitud;
  db.tickets[ticketIdx].completada = completada ? true : false;

  fs.writeFile('./db.json', JSON.stringify(db, null, 2), () => {} );
  res.redirect("/");
})

app.listen(3000, () => {
  console.log('Server started: http://localhost:3000/');
});
