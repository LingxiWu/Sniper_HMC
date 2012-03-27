#include <math.h>
#include <stdio.h>

#include "simulator.h"
#include "stats.h"
#include "utils.h"
#include "itostr.h"

void
StatsManager::registerMetric(StatsMetricBase *metric)
{
   m_objects.push_back(metric);
}

StatsMetricBase *
StatsManager::getMetricObject(String objectName, UInt32 index, String metricName)
{
   for(std::vector<StatsMetricBase *>::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
      if (   (*it)->objectName == objectName
          && (*it)->index == index
          && (*it)->metricName == metricName
      )
         return *it;
   return NULL;
}

StatsManager::StatsManager()
   : m_fp(NULL)
{
}

StatsManager::~StatsManager()
{
   if (m_fp)
      fclose(m_fp);
}

void
StatsManager::recordStats(String prefix, String fileName)
{
   if (fileName == "") {
      if (! m_fp) {
         String filename;
         UInt32 pnum = Sim()->getConfig()->getCurrentProcessNum();
         if (pnum)
            filename = "sim-" + itostr(pnum) + ".stats";
         else
            filename = "sim.stats";
         filename = Sim()->getConfig()->formatOutputFileName(filename);
         m_fp = fopen(filename.c_str(), "w");
         LOG_ASSERT_ERROR(m_fp, "Cannot open %s for writing", filename.c_str());
      }
      recordStats(prefix, m_fp);
   } else {
      String filename = Sim()->getConfig()->formatOutputFileName(fileName);
      FILE *fp = fopen(filename.c_str(), "a");
      LOG_ASSERT_ERROR(fp, "Cannot open %s for writing", filename.c_str());
      recordStats(prefix, fp);
      fclose(fp);
   }
}

void
StatsManager::recordStats(String prefix, FILE *fp)
{
   for(std::vector<StatsMetricBase *>::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
      fprintf(fp, "%s.%s[%u].%s %s\n", prefix.c_str(), (*it)->objectName.c_str(), (*it)->index, (*it)->metricName.c_str(), (*it)->recordMetric().c_str());
   fflush(fp);
}


StatHist &
StatHist::operator += (StatHist & stat)
{
   if (n == 0) { min = stat.min; max = stat.max; }
   n += stat.n;
   s += stat.s;
   s2 += stat.s2;
   if (stat.n && stat.min < min) min = stat.min;
   if (stat.n && stat.max > max) max = stat.max;
   for(int i = 0; i < HIST_MAX; ++i)
      hist[i] += stat.hist[i];
   return *this;
}

void
StatHist::update(unsigned long v)
{
   if (n == 0) {
      min = v;
      max = v;
   }
   n++;
   s += v;
   s2 += v*v;
   if (v < min) min = v;
   if (v > max) max = v;
   int bin = floorLog2(v) + 1;
   if (bin >= HIST_MAX) bin = HIST_MAX - 1;
      hist[bin]++;
}

void
StatHist::print()
{
   printf("n(%lu), avg(%.2f), std(%.2f), min(%lu), max(%lu), hist(%lu",
      n, n ? s/float(n) : 0, n ? sqrt((s2/n - (s/n)*(s/n))*n/float(n-1)) : 0, min, max, hist[0]);
   for(int i = 1; i < HIST_MAX; ++i)
      printf(",%lu", hist[i]);
   printf(")\n");
}