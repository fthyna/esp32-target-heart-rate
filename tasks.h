#ifndef TASKS_H
#define TASKS_H

void dataCollectionTask(void *param);
void dataProcessingTask(void *param);
void dataUploadTask(void *param);

void initializeTasks();

#endif