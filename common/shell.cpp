#ifndef SOCROCKET_SHELL_H
#define SOCROCKET_SHELL_H

#include <Python.h>
#include <systemc.h>
#include "shell.h"

using namespace sc_core;

static PyObject *sysc_start(PyObject *self, PyObject *args) {
  if(!PyArg_ParseTuple(args, ":start")) {
    return NULL;
  }
  sc_start();
  return Py_BuildValue("");
}

static PyObject *sysc_pause(PyObject *self, PyObject *args) {
  if(!PyArg_ParseTuple(args, ":pause")) {
    return NULL;
  }
  sc_pause();
  return Py_BuildValue("");
}

static PyObject *sysc_stop(PyObject *self, PyObject *args) {
  if(!PyArg_ParseTuple(args, ":exit")) {
    return NULL;
  }
  sc_stop();
  return Py_BuildValue("");
}

static PyMethodDef GSMethods[] = {
  {"list", &sysc_start, METH_VARARGS, "List all gs_params."}, // Not Implemented
  {"get_param", &sysc_stop, METH_VARARGS, "Get gs_param value."}, // Not Implemented
  {NULL, NULL, 0, NULL}
};

static PyMethodDef SysCMethods[] = {
  {"start", &sysc_start, METH_VARARGS, "Continue simulation."},
  {"pause", &sysc_pause, METH_VARARGS, "Pause simulation."},
  {"stop", &sysc_stop, METH_VARARGS, "Stop simulation."},
  {NULL, NULL, 0, NULL}
};

void start_shell(sc_status status) {
  int argc = 1;
  char *argv[1] = {"SystemC"};
  Py_SetProgramName(argv[0]);
  Py_Initialize();

  PyImport_AddModule("systemc");
  Py_InitModule("systemc", SysCMethods);

  PyImport_AddModule("greenlib");
  Py_InitModule("greenlib", GSMethods);

  while(1) {
    if(status == SC_RUNNING) {
      sc_start();
      status = sc_get_status();
    } else if(status == SC_PAUSED) {
      Py_Main(argc, argv);
    } else {
      break;
    }
  }

  Py_Finalize();
}

#endif // SOCROCKET_SHELL_H
