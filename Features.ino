// -----------------------------------------------------------------------------------
// Auxiliary Features

#ifdef FEATURES_PRESENT
void featuresInit() {
  for (int i=0; i < 8; i++) {
    if (feature[i].purpose == SWITCH || feature[i].purpose == ANALOG) {
      if (feature[i].pin >= 0 && feature[i].pin <= 255) pinMode(feature[i].pin,OUTPUT);
    } else if (feature[i].purpose == DEW_HEATER) {
      feature[i].dewHeater = new dewHeaterControl;
      // with ambient temperature: zero = -5 (dew forming 100% power), span = 15 (dew NOT forming 0% power)
      if (feature[i].pin >= 0 && feature[i].pin <= 255) feature[i].dewHeater->init(feature[i].pin,EE_feature1Value1+i*3); else feature[i].dewHeater->init(-1,EE_feature1Value1+i*3);
    }
  }
}

void featuresPoll() {
#ifdef FEATURES_PRESENT
  for (int i=0; i < 8; i++) {
    if (feature[i].purpose == DEW_HEATER) {
      feature[i].dewHeater->poll(ambient.getFeatureTemperature(0)-ambient.getDewPoint());
      if ((feature[i].pin & DS_MASK) == DS2413) ambient.setDS2413State(i,feature[i].dewHeater->isOn());
    }
  }
#endif
}

// :GXXn#
void featuresGetCommand(char *parameter, char *reply, bool &booleanReply) {
  int i=parameter[1]-'1';
  if (i < 0 || i > 7)  { commandError=CE_PARAM_FORM; return; }

  char s[255];
  if (feature[i].purpose == SWITCH) {
    if (feature[i].pin >= 0 && feature[i].pin <= 255) sprintf(s,"%d",feature[i].value); else sprintf(s,"%d",ambient.getDS2413State(i)); strcat(reply,s);
  } else if (feature[i].purpose == ANALOG) {
    if (feature[i].pin >= 0 && feature[i].pin <= 255) sprintf(s,"%d",feature[i].value); else strcpy(s,"0"); strcat(reply,s);
  } else if (feature[i].purpose == DEW_HEATER) {
    sprintf(s,"%d",(int)feature[i].dewHeater->isEnabled()); strcat(reply,s); strcat(reply,",");
    dtostrf(feature[i].dewHeater->getZero(),3,1,s); strcat(reply,s); strcat(reply,",");
    dtostrf(feature[i].dewHeater->getSpan(),3,1,s); strcat(reply,s); strcat(reply,",");
    dtostrf(ambient.getFeatureTemperature(i)-ambient.getDewPoint(),3,1,s); strcat(reply,s);
  } else { commandError=CE_CMD_UNKNOWN; return; }
  booleanReply=false;
}

// :GXYn#
void featuresGetInfoCommand(char *parameter, char *reply, bool &booleanReply) {
  int i=parameter[1]-'1';
  if (i < 0 || i > 7)  { commandError=CE_PARAM_FORM; return; }
  if (feature[i].purpose == OFF) { commandError=CE_CMD_UNKNOWN; return; }
  
  char s[255];
  strcpy(s,feature[i].name); if (strlen(s)>10) s[10]=0; strcpy(reply,s); strcat(reply,",");
  sprintf(s,"%d",(int)feature[i].purpose); strcat(reply,s);
  booleanReply=false;
}

// :SXX[n],V[Z][S][v]#
// for example :SXX1,V1#  :SXX1,Z0.5#
void featuresSetCommand(char *parameter) {
  int i=parameter[1]-'1';
  if (i < 0 || i > 7)  { commandError=CE_PARAM_FORM; return; }
  if (feature[i].purpose == OFF) { commandError=CE_CMD_UNKNOWN; return; }
  
  char* conv_end;
  double f=strtod(&parameter[4],&conv_end);
  if (&parameter[4] == conv_end) { commandError=CE_PARAM_FORM; return; }
  long v = lround(f);

  if (v >= 0 && v <= 255) feature[i].value=v;
  if (feature[i].purpose == SWITCH) {
    if (parameter[3] == 'V') {
      if (v >= 0 && v <= 1) {
        if (feature[i].pin >= 0 && feature[i].pin <= 255) digitalWrite(feature[i].pin,v==0?LOW:HIGH); else ambient.setDS2413State(feature[i].pin,v==0?0:1);
      } else commandError=CE_PARAM_RANGE;
    } else commandError=CE_PARAM_FORM;
  } else if (feature[i].purpose == ANALOG) {
    if (parameter[3] == 'V') {
      if (v >= 0 && v <= 255) {
        if (feature[i].pin >= 0 && feature[i].pin <= 255) analogWrite(feature[i].pin,v);
      } else commandError=CE_PARAM_RANGE;
    } else commandError=CE_PARAM_FORM;
  } else if (feature[i].purpose == DEW_HEATER) {
    if (parameter[3] == 'V') {
      if (v >= 0 && v <= 1) feature[i].dewHeater->enable(v); else commandError=CE_PARAM_RANGE;
    } else if (parameter[3] == 'Z') {
      if (f >= -5.0 && f <= 20.0) feature[i].dewHeater->setZero(f); else commandError=CE_PARAM_RANGE;
    } else if (parameter[3] == 'S') {
      if (f >= -5.0 && f <= 20.0) feature[i].dewHeater->setSpan(f); else commandError=CE_PARAM_RANGE;
    } else commandError=CE_PARAM_FORM;
  }
}
#endif