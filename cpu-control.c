#include <math.h>
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static float TEMP_NORM = 50;
static float TEMP_TOLE = 70;
static float TEMP_CRIT = 90;

float rnum(const char *path);
float temp();
float freq();
char *gov();
void setf(float value);

int main(int ac, char *av[])
{
  while (1)
  {
    sleep(1);
    const float minfreq = rnum("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq");
    const float maxfreq = rnum("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (!strcmp(gov(), "powersave"))
      continue;

    float t = temp();

    if (t >= TEMP_TOLE)
      setf(fmaxf(minfreq, freq() - (maxfreq - minfreq) * 0.1 * (t - TEMP_TOLE) / (TEMP_CRIT - TEMP_TOLE)));
    else if (t >= TEMP_NORM)
      setf(fminf(maxfreq, freq() + (maxfreq - minfreq) * 0.1 * (t - TEMP_NORM) / (TEMP_TOLE - TEMP_NORM)));
    else
      setf(maxfreq);
  }
}

void setf(float value)
{
  glob_t g;
  memset(&g, 0, sizeof(glob_t));
  if (!glob("/sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq", GLOB_DOOFFS, NULL, &g))
  {
    for (int i = 0; i < g.gl_pathc; ++i)
    {
      FILE *file;
      if (file = fopen(g.gl_pathv[i], "w"))
      {
        fprintf(file, "%d", (int)(value));
        fclose(file);
      }
    }
    globfree(&g);
  }
}

char *gov()
{
  static char res[32];
  memset(res, 0, 32);
  FILE *file;
  if (file = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "r"))
  {
    fscanf(file, "%[a-z]", res);
    fclose(file);
  }
  return res;
}

float rnum(const char *path)
{
  float res = 0;
  FILE *file;
  if (file = fopen(path, "r"))
  {
    fscanf(file, "%f", &res);
    fclose(file);
  }
  return res;
}

float freq()
{
  return rnum("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
}

float temp()
{
  float res = 0;
  glob_t g;
  memset(&g, 0, sizeof(glob_t));
  if (!glob("/sys/class/thermal/thermal_zone*/temp", GLOB_DOOFFS, NULL, &g))
  {
    for (int i = 0; i < g.gl_pathc; ++i)
    {
      float tmp = rnum(g.gl_pathv[i]);
      if (tmp > res)
        res = tmp;
    }
    globfree(&g);
  }
  return res / 1000;
}