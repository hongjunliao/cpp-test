# src/main/resources/application.yml:
server:
  port: 8082
spring:
  application:
    name: sidecar-c

eureka:
  client:
    service-url:
      defaultZone: http://localhost:8888/eureka/ # 服务注册中心地址

sidecar:
  port: 8100
  health-uri: http://localhost:8100/health.json
# pom.xml:
<?xml version="1.0" encoding="UTF-8"?>
<project xmlns="http://maven.apache.org/POM/4.0.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://maven.apache.org/POM/4.0.0 http://maven.apache.org/xsd/maven-4.0.0.xsd">
    <modelVersion>4.0.0</modelVersion>

    <groupId>springcloud</groupId>
    <artifactId>springcloud-eureka-client-sidecar</artifactId>
    <version>0.0.1-SNAPSHOT</version>
    <name>springcloud-eureka-client-sidecar::sidecar</name>

    <!-- Spring Boot 启动父依赖 -->
    <parent>
        <groupId>org.springframework.boot</groupId>
        <artifactId>spring-boot-starter-parent</artifactId>
        <version>1.5.4.RELEASE</version>
    </parent>

    <dependencies>
        <!-- Spring Cloud Netflix Eureka 依赖 -->
        <dependency>
            <groupId>org.springframework.cloud</groupId>
            <artifactId>spring-cloud-starter-eureka</artifactId>
        </dependency>

        <!-- Spring Boot Test 依赖 -->
        <dependency>
            <groupId>org.springframework.boot</groupId>
            <artifactId>spring-boot-starter-test</artifactId>
            <scope>test</scope>
        </dependency>
		<!--sidecar-->
<dependency>
	    <groupId>org.springframework.cloud</groupId>
		    <artifactId>spring-cloud-netflix-sidecar</artifactId>
		</dependency>
    </dependencies>

    <dependencyManagement>
        <dependencies>
            <!-- Spring Cloud Netflix 依赖 -->
            <dependency>
                <groupId>org.springframework.cloud</groupId>
                <artifactId>spring-cloud-netflix</artifactId>
                <version>1.3.1.RELEASE</version>
                <type>pom</type>
                <scope>import</scope>
            </dependency>
        </dependencies>
    </dependencyManagement>

    <build>
        <plugins>
            <plugin>
                <groupId>org.apache.maven.plugins</groupId>
                <artifactId>maven-compiler-plugin</artifactId>
                <configuration>
                    <source>1.8</source>
                    <target>1.8</target>
                </configuration>
            </plugin>
        </plugins>
    </build>

</project>
# src/main/java/org/spring/springcloud/SampleSidecarApplication.java:
package org.spring.springcloud;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.cloud.netflix.sidecar.EnableSidecar;
 
@SpringBootApplication
@EnableSidecar
public class SampleSidecarApplication {
 
    public static void main(String[] args) {
        SpringApplication.run(SampleSidecarApplication.class, args);
    }
}
