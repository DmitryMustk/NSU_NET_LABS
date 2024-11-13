plugins {
	java
}

group = "ru.nsu.dmustakaev"
version = "0.0.1-SNAPSHOT"

java {
	toolchain {
		languageVersion = JavaLanguageVersion.of(21)
	}
}

configurations {
	compileOnly {
		extendsFrom(configurations.annotationProcessor.get())
	}
}

repositories {
	mavenCentral()
}

dependencies {
//	compileOnly("org.projectlombok:lombok:1.18.28")
//	annotationProcessor("org.projectlombok:lombok:1.18.28")

	implementation("org.slf4j:slf4j-api:2.0.16")

	implementation("dnsjava:dnsjava:3.6.2")

//	testImplementation("org.junit.jupiter:junit-jupiter")
}
//
//tasks.withType<Test> {
//	useJUnitPlatform()
//}
