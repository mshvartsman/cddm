library(stringr)
library(data.table)
library(ggplot2)
library(gridExtra)
library(plyr)

binpath <- '../bin/'



####### FLANKER ########
flankerpar <- 'timePerStep=1,maxTrials=100000,maxSamps=10000,contextNoise=9,targetNoise=9,decisionThresh=0.9,eblMean=0,motorPlanMean=0,motorExecMean=0,eblSd=1,motorSd=1,trialDist=0.5 0; 0.5 0,urPrior=0.45 0.05; 0.05 0.45,nContexts=2,nTargets=2,pPrematureResp=0.03,contextMeanSpacing=1,targetMeanSpacing=1'


# run the flanker model
print("Running flanker model...")
flankerCmd <- sprintf('echo "%s" | %sflanker_trace', flankerpar, binpath)
system(flankerCmd) # this will take a few minutes

accfiles <- dir('./flankerTrace_output', "Acc", full.names=T)
acctraces <- vector("list", length(accfiles))

for (i in 1:length(accfiles)){
  if (file.info(accfiles[i])$size!=0){
    labels <- str_match(accfiles[i], "Context(.)_Target(.).*")[,2:3]
    tmp <- fread(accfiles[i])
    tmp <- cbind(tmp, t(labels))
    acctraces[[i]] <- tmp
  }
}
acctraces <- rbindlist(acctraces)
setnames(acctraces, c("Trial","Acc","Context","Target"))

rtfiles <- dir('./flankerTrace_output', "_RT", full.names=T)
rttraces <- vector("list", length(rtfiles))

for (i in 1:length(rtfiles)){
  if (file.info(rtfiles[i])$size!=0){
    labels <- str_match(rtfiles[i], "Context(.)_Target(.).*")[,2:3]
    tmp <- fread(rtfiles[i])
    tmp <- cbind(tmp, t(labels))
    rttraces[[i]] <- tmp
  }
}
rttraces <- rbindlist(rttraces)
setnames(rttraces, c("Trial","RT","Context","Target"))

setkey(acctraces, Trial, Context, Target)
setkey(rttraces, Trial, Context, Target)

summarytraces <- rttraces[acctraces]

print("Generating flanker figures...")
cutsize <- 50
summarytraces[,binRT:=(RT %/% cutsize) * cutsize]
summarytraces[,binsize:=length(RT),by="binRT"]
summarytraces[,Congruence:=ifelse(Context==Target,"Congruent","Incongruent")]
p <- ggplot(summarytraces[binsize>=50], aes(x=binRT, y=Acc, group=Congruence, colour=Congruence)) + stat_summary(fun.data="mean_cl_normal") + stat_summary(fun.y="mean", geom="line") + ylim(c(0,1)) + geom_hline(yintercept=0.5, linetype="dashed") + theme_bw(base_size=8) + labs(x="Timesteps",y="Accuracy") + xlim(c(0,1000)) + theme(legend.position=c(0.80,0.25), plot.margin = rep(unit(0,"null"),4), legend.text=element_text(size=6),legend.title=element_blank(), legend.key.size = unit(0.5, "cm"), legend.background=element_blank()) 
p
ggsave(p, filename="flanker_conditionalRT.pdf", width=2.75, height=1.375)

p <- ggplot(summarytraces[binsize>=50], aes(x=RT, group=Congruence, colour=Congruence)) +geom_density()+ theme_bw(base_size=8) + labs(x="Timesteps") + xlim(c(0,1000)) + theme(legend.position=c(0.8,0.7), plot.margin = rep(unit(0,"null"),4),legend.title=element_blank(),legend.text=element_text(size=6),legend.title=element_blank(), legend.key.size = unit(0.5, "cm")) 
p
ggsave(p, filename="flanker_rtDist.pdf", width=2.75, height=1.375)


###### AX-CPT #####
print("Running AX-CPT model...")
axcptpar <- 'timePerStep=1,maxTrials=100000,maxSamps=10000,contextNoise=9,targetNoise=9,decisionThresh=0.9,eblMean=0,motorPlanMean=0,motorExecMean=0,eblSd=1,motorSd=1,trialDist=0.5 0.2; 0.2 0.1,urPrior=0.5 0.2; 0.2 0.1,nContexts=2,nTargets=2,pPrematureResp=0.03,contextMeanSpacing=1,targetMeanSpacing=1,decayRate=0.0001'
axcptCmd <- sprintf('echo "%s" | %saxcpt_trace', axcptpar, binpath)
system(axcptCmd) # this will take a few minutes

accfiles <- dir("./axcptTrace_Output", "Acc", full.names=T)
acctraces <- vector("list", length(accfiles))

for (i in 1:length(accfiles)){
  if (file.info(accfiles[i])$size!=0){
    labels <- str_match(accfiles[i], "Context(.)_Target(.).*")[,2:3]
    tmp <- fread(accfiles[i])
    tmp <- cbind(tmp, t(labels))
    acctraces[[i]] <- tmp
  }
}
acctraces <- rbindlist(acctraces)
setnames(acctraces, c("Trial","Acc","Context","Target"))

rtfiles <- dir("./axcptTrace_Output", "_RT", full.names=T)
rttraces <- vector("list", length(rtfiles))

for (i in 1:length(rtfiles)){
  if (file.info(rtfiles[i])$size!=0){
    labels <- str_match(rtfiles[i], "Context(.)_Target(.).*")[,2:3]
    tmp <- fread(rtfiles[i])
    tmp <- cbind(tmp, t(labels))
    rttraces[[i]] <- tmp
  }
}
rttraces <- rbindlist(rttraces)
setnames(rttraces, c("Trial","RT","Context","Target"))

setkey(acctraces, Trial, Context, Target)
setkey(rttraces, Trial, Context, Target)

summarytraces <- rttraces[acctraces]
print("Generating AX-CPT figures...")
cutsize <- 50
summarytraces[,binRT:=(RT %/% cutsize) * cutsize]
summarytraces[,binsize:=length(RT),by="binRT"]
summarytraces[,Congruence:=ifelse(Context==Target,"Congruent","Incongruent")]
summarytraces[,contextName:=ifelse(Context==0,"A","B")]
summarytraces[,targetName:=ifelse(Target==0,"X","Y")]
summarytraces[,TrialType:=factor(contextName):factor(targetName)]
p <- ggplot(summarytraces[binsize>=50], aes(x=binRT, y=Acc, group=TrialType, colour=TrialType))+ geom_hline(yintercept=0.5, linetype="dashed", alpha=0.5)+ stat_summary(fun.data="mean_cl_normal",alpha=0.9) + stat_summary(fun.y="mean", geom="line",alpha=0.6) + ylim(c(0,1))  + xlim(c(0,1000)) + theme_bw(base_size=6) + labs(x="Timesteps",y="Accuracy", colour="Trial Type", title="Conditional Accuracy (model)")  + theme(legend.position=c(0.89,0.25), plot.margin = rep(unit(0,"null"),4),legend.title=element_blank(), legend.background=element_blank(), legend.text=element_text(size=5), legend.key.size = unit(0.2, "cm")) 
p
ggsave(p, filename="axcpt_conditionalRT.pdf", width=2.75, height=1.375)

pErr <- ggplot(summarytraces[binsize>=50], aes(x=TrialType, y=1-Acc)) + stat_summary(fun.data="mean_cl_normal") + theme_bw(base_size=10) + labs(x="Trial Type",y="Error Proportion",title="Errors by condition (model)")  + theme(legend.position=c(0.3,0.25), plot.margin = rep(unit(0,"null"),4)) + coord_cartesian(ylim=c(0.04,0.22))
pErr

pRT <- ggplot(summarytraces[binsize>=50], aes(x=TrialType, y=RT) )+ stat_summary(fun.data="mean_cl_normal") + theme_bw(base_size=10) + labs(x="Trial Type",y="RT (timesteps)",title="RT by condition (model)")  + theme( plot.margin = rep(unit(0,"null"),4),legend.title=element_blank())
pRT

pdf(file="axcpt_conditionalMeans.pdf", width=2.75, height=2.75)
grid.arrange(pRT,pErr, ncol=1)
dev.off()

#### For human data, contact Olga Lositsky at lositsky@princeton.edu
