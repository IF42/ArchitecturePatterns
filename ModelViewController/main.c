#include <stdio.h>
#include <stdlib.h>


/************ Controlled quantities ************/
typedef struct {
    int intenzity;
    /* other parameters */
} FanController;


#define FanController() (FanController) {0}


void fan_controller_set_intenzity(FanController * self, int intenzity) {
    if(intenzity >= 0 && intenzity <= 100) { 
        self->intenzity = intenzity;
    }
}


typedef struct {
    int flow;
    /* other parameters */
}WaterValveController;


#define WaterValveController()(WaterValveController) {0}


void water_valve_controller_set_flow(WaterValveController * self, int flow) {
    if(flow >= 0 && flow <= 100) { 
        self->flow = flow;
    }
}

/*
 * Model class definition 
 * Model contains each controlled module as independent part and creates safe interface for its access
 * Model also managing data input from other sources than from user
 */
typedef struct {
    FanController fan;
    WaterValveController water_valve;
    int ATS;
}Model;


#define Model()(Model){.fan=FanController(), .water_valve=WaterValveController()}


int model_water_valve_flow(Model * self) {
    return self->water_valve.flow;
}


int model_fan_intenzity(Model * self) {
    return self->fan.intenzity;
}


int model_ats(Model * self) {
    return self->ATS;
}


/*
 * simulation of ATS reading
 */
void model_read_ats(Model * self, int ATS) {
    self->ATS = ATS;
}


/*
 * View class definition
 * View creates inteface for inputs and outputs and interaction witch user or other system. 
 */
typedef struct {
    Model * model;
    /* other parameters */
}View;


#define View(_model) (View) {.model=(_model)}


void view_display(View * self) {
    printf("ATS: %d°C\n", model_ats(self->model));
    printf("Water Valve: %d\%%\n", model_water_valve_flow(self->model));
    printf("Fan intenzity: %d\%%\n", model_fan_intenzity(self->model));
    printf("\n");
}


/*
 * Controller class definition
 * Controller class controles all modules each other and ensuer its interconnection and data exchange
 *  
 */
typedef struct {
    Model * model;
    int state;
}Controller;


#define Controller(_model) (Controller){.model=(_model), .state=0}


void controller_transition(Controller * self) {
    /*
     * automatic water valve flow control based on ATS temperature
     */
    switch(self->state) {
        case 0:
            if(model_ats(self->model) > 20) {
                self->state = 1;
            } else if(model_ats(self->model) < 0) {
                self->state = 2;
            }

            water_valve_controller_set_flow(&self->model->water_valve, 50);

            break;
        case 1:
            if(model_ats(self->model) < 15) {
                self->state = 0;
            }

            water_valve_controller_set_flow(&self->model->water_valve, 25);
            break;
        case 2:
            if(model_ats(self->model) > 5) {
                self->state = 0;
            }

            water_valve_controller_set_flow(&self->model->water_valve, 100);
            break;
        default:
            self->state = 0;
    }

    /*
     * Automatic fan intenzity control based on water valve flow
     */
    if(model_water_valve_flow(self->model) == 0) {
        fan_controller_set_intenzity(&self->model->fan, 0);
    } else if(model_water_valve_flow(self->model) > 0 && model_water_valve_flow(self->model) < 30) {
        fan_controller_set_intenzity(&self->model->fan, 40);
    } else if(model_water_valve_flow(self->model) >= 30 && model_water_valve_flow(self->model) < 70) {
        fan_controller_set_intenzity(&self->model->fan, 65);
    } else if(model_water_valve_flow(self->model) >= 70) {
        fan_controller_set_intenzity(&self->model->fan, 100);
    }
}


static Model model;
static View view;
static Controller controller;
static int ATS_IN = -10;


static void setup(void) {
    model = Model();
    view = View(&model);
    controller = Controller(&model);
}


static void loop(void) {
    model_read_ats(&model, ATS_IN);
    controller_transition(&controller);
    view_display(&view);
}


int main(void) {
    /*
     * application initialization
     */
    setup();

    /*
     * main controll loop
     */
    for(int i = 0; i < 10; i++) {
        loop();
        ATS_IN += 5;
    }

    printf("Program exit..\n");
    return EXIT_SUCCESS;
}






