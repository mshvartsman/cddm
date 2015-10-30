library(ggplot2)
library(reshape2)
library(plyr)
library(data.table)

csvFiles <- list.files('.', pattern='.csv')



traces.wide <- data.frame()
events.all <- data.frame()
trialLevel.all <- data.frame()

for (f in 1:length(csvFiles)){
  if (file.info(csvFiles[f])$size != 0){
    if(grepl("post", csvFiles[f])==T){ # traces
      tmp <- read.csv(csvFiles[f], header=F)
      #tmp[,1] <- tmp[,1] + (f-1)
      tmp$file = csvFiles[f]
      traces.wide <- rbind(traces.wide, tmp)  
    } else if(grepl("Event", csvFiles[f])==T){ # events
      tmp <- read.csv(csvFiles[f], header=F)
      #tmp[,1] <- tmp[,1] + (f-1)
      tmp$file = csvFiles[f]
      events.all <- rbind(events.all, tmp)  
    } else{ # trial-level summaries
      tmp <- read.csv(csvFiles[f], header=F)
      #tmp[,1] <- tmp[,1] + (f-1)
      tmp$file = csvFiles[f]
      trialLevel.all <- rbind(trialLevel.all, tmp)  
    }
  }  
}

# process events
colnames(events.all) <- c("traceID", "start", "end", "filename")
events.all$name <- sub("Context._Target._(\\w*)Event.csv", "\\1", events.all$filename)
events.all$trueTrialType <- sub("Context(.)_Target(.)_.*.csv", "\\1\\2", events.all$filename)
events.all$trueTrialType  <- mapvalues(events.all$trueTrialType, from=c("00", "01", "10", "11"), to = c("AX", "AY", "BX", "BY"))

events.all$filename <- NULL
events.all <- as.data.table(events.all)

# process trialLevel trace
colnames(trialLevel.all) <- c("traceID", "val", "filename")
trialLevel.all$name <- sub("Context._Target._(\\w*).csv", "\\1", trialLevel.all$filename)
trialLevel.all$trueTrialType <- sub("Context(.)_Target(.)_.*.csv", "\\1\\2", trialLevel.all$filename)
trialLevel.all$trueTrialType  <- mapvalues(trialLevel.all$trueTrialType, from=c("00", "01", "10", "11"), to = c("AX", "AY", "BX", "BY"))
trialLevel.all$filename <- NULL
trialLevel.all <- as.data.table(trialLevel.all)
# process traces trace
# AX BX AY BY rather than AX AY BX BY because arma::vectorise gives us column-wise by default
colnames(traces.wide) <- c("traceID", "time", "AX","BX","AY","BY", "filename")

traces.wide$L <- traces.wide$AX + traces.wide$BY
traces.wide$R <- traces.wide$AY + traces.wide$BX

traces.wide$trueTrialType <- mapvalues(traces.wide$filename, from=c("Context0_Target0_post.csv", "Context0_Target1_post.csv", "Context1_Target0_post.csv", "Context1_Target1_post.csv"), to = c("AX", "AY", "BX", "BY"))
traces.wide$filename <- NULL

traces.all <- melt(traces.wide, id.vars = c("traceID","time","trueTrialType"), variable.name = "beliefTrialType", value.name = "prob")

traces.all <- as.data.table(traces.all)

colnames(events.all)

head(traces.all)

traces.all[,cresp:=mapvalues(trueTrialType, from=c("AX","AY","BX","BY"), to=c("L","R","R","L"))]
traces.all[,resp:=ifelse(max(prob[beliefTrialType=="L"])>0.5, "L", "R"),by="traceID,trueTrialType"]

events.all[,cresp:=mapvalues(trueTrialType, from=c("AX","AY","BX","BY"), to=c("L","R","R","L"))]

trialLevel.all[,cresp:=mapvalues(trueTrialType, from=c("AX","AY","BX","BY"), to=c(1,0,0,1))]


# p <- ggplot(traces[ beliefTrialType == "L"], aes(x=time, y=prob, group=beliefTrialType, colour=beliefTrialType)) + geom_line() + facet_wrap(~traceID)
# p


traces <- traces.all[traceID==0]
events <- events.all[traceID==0]
trialLevel <- trialLevel.all[traceID==0]

plotEvent <- function(bounds, name, ystart=1, height=0.5){
  event.bot <- ystart
  event.top <- event.bot + height
  event.col <- rgb(0,0,0,0.3)
  text(label=name, srt=90,cex=0.6, x=-10, y = event.bot + (event.top-event.bot)/2)
  if (nrow(bounds)==0) return()
  for (bd in 1:nrow(bounds)) {
    s <- bounds[bd,1]
    e <- bounds[bd,2]
    rect(s, event.bot, e, event.top,col=event.col, border=NA)
  }
}

markSampling <- function(bounds, name, col, thresh){
  event.bot <- c(0, 1.02)
  event.top <- c(0.99, 2)
  event.col <- col
  

  
  if (nrow(bounds)==0) return()
  for (bd in 1:nrow(bounds)) {
    s <- bounds[bd,1]
    e <- bounds[bd,2]
    rect(s, event.bot, e, event.top, col=event.col, border=NA)
  }
  # overplot thresh
  abline(h=thresh, lty=2)
  text(label=name,cex=0.6, x=bounds[1,2]-bounds[1,1], y = 0.01)
}

setupPlot <- function(accLabel, trialLabel, maxTime, thresh, paramString = ""){
  plot(NA, type="p",
       cex=0.4, pch=20,
       lwd=2,
       bty="n",
       yaxt="n",
       ylim=c(0,4.1),
       xlim=c(0,maxTime),
       ylab=NA,
       xlab="Trial Time (ms)",
       main=sprintf("%s %s Trial",accLabel,trialLabel),
       sub = paramString)
  ## Backdrop for LR posterior
  rect(0,0,maxTime,0.99,col="grey90",border=NA)
  ## Backdrop for ABYX plosterior
  rect(0,1.01,maxTime,2,col="grey90",border=NA)
#   Threshold lines
#   lines(x=c(0,maxTime),y=c(thresh, thresh),lty=2)
}

staticPlot <- function(traces, events, trialLevel, config){
  lrBeliefTrace <- traces[beliefTrialType=="L"]
  abxyBeliefTrace <- traces[!(beliefTrialType %in% c("L","R"))]
  response <- trialLevel[name=="Resp"]$val
  maxTime <- max(events$end)
  if(response == -1){
    responseLabel <- "No response"
    motorStart <- maxTime-200
  } else {
    motorStart <- events[name=="motor"]$start
    motorEnd <- events[name=="motor"]$end
    if(response==0){
      responseLabel <- "Right"
    } else if (response==1){
      responseLabel <- "Left"
    }
  }
  trialLabel <- trialLevel$trueTrialType[1]
  accLabel <- ifelse(trialLevel[name=="Acc"]$val[1]==1, "Correct", "Incorrect")
  setupPlot(accLabel=accLabel,trialLabel=trialLabel,maxTime=maxTime+300,thresh=config$thresh)
  sampColors <- c(samplingContext="darkseagreen3", samplingBoth="thistle3")
  for (ev in grep("sampling", unique(events$name), value=T)){
    bounds <- cbind(events[name==ev]$start, events[name==ev]$end)
    markSampling(bounds, ev, sampColors[ev], config$thresh)
  }
  
  # plot beliefs
  lines(lrBeliefTrace$time, lrBeliefTrace$prob)
  text(label="L",x=maxTime+200,y = 0.8,cex=0.85)
  text(label="R",x=maxTime+200,y = 0.2,cex=0.85)
  belColors <- c("darkred", "darkblue", "darkgreen", "orange")
  names(belColors) <- list("AX", "AY", "BX", "BY")
  tmp <- 0
  for (bel in unique(abxyBeliefTrace$beliefTrialType)){
    lines(abxyBeliefTrace[beliefTrialType==bel]$time, abxyBeliefTrace[beliefTrialType==bel]$prob+1, col=belColors[bel])
    text(label=bel,x=maxTime+200,y = 1.15+tmp,col=belColors[bel], cex=0.75)
    tmp <- tmp + 0.25
  }
  # plot events
  currentY <- 2.05
  for (ev in grep("sampling", unique(events$name), value=T, invert=T)){
    bounds <- cbind(events[name==ev]$start, events[name==ev]$end)
    plotEvent(bounds, ev, ystart=currentY, height=0.75)
    currentY <- currentY + 0.77
  } 
}
config <- list()
config$thresh <- 0.85
pdf("tracePlots_axcpt.pdf")
for (tid in unique(traces.all$traceID)){
  staticPlot(traces.all[traceID==tid], events.all[traceID==tid], trialLevel.all[traceID==tid], config)
}
dev.off()